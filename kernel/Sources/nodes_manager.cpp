#include <nodes_manager.hpp>
#include <string.hpp>
#include <debug.hpp>

/// @brief Initializes the variables of class
/// @param start_address start address of memory
/// @param total_usable_mem size of total usable memory
/// @param memmap_count number of memory maps
/// @param memmap memory maps structure
void memory::NodesManager::init(max_t start_address , max_t end_address) {
	node_start = (struct Node *)start_address;   // StartAddress 	       : The location of the memory pool
	maximum_node_addr = 0x00;
	mem_start_address = start_address;
	mem_end_address = end_address;
	currently_using_mem = 0;					   // CurrentlyUsingMemory : Currently using memory size
	allocation_available = true;
}

/// @brief Gives boolean value of whether manager is available
/// @return Return true if manager is available to allot
bool memory::NodesManager::available(void) {
	return allocation_available;
}

/// @brief Allocate physical kernel memory
/// @param size Size to allocate
/// @param alignment Alignment
/// @return 
max_t memory::NodesManager::allocate(max_t size , max_t alignment) {
	default_t i;
	max_t address;
	max_t total_node_size = 0;	// TotalNodeSize : Size of the total node that is going to be used for allocation
	// Load the node_mgr from the local address
	if(size == 0) return 0x00;
	if(!allocation_available) return 0x00; // no available node
	struct Node *node = (struct Node *)search_first_fit(size); // Search available node
	struct Node *separated_node;
	if(node == 0x00) { // If we got to create new node, create new node at the end of the segments
		node = create_new_node(size , alignment);
		if(node == 0x00) return 0x00; // not available
		total_node_size = 0;								// Set the value to 0 so that the <Node Seperation Sequence> can't be executed.	
	}
	else {
		if(node->next != 0) {			// If the next node is present, set TotalNodeSize to the size of searched node.
			total_node_size = ((max_t)node->next)-((max_t)node)-sizeof(struct Node);	// TotalNodeSize : size of the current node
		}
		if(alignment != 0) { // is it not aligned
			// Search New alignable location
			// printf("Alignment : %d\n" , Alignment);
			node = (struct Node *)search_aligned(size , alignment);
			// printf("Found aligned, new one : 0x%X\n" , Node);
			if(node == 0x00) node = create_new_node(size , alignment);
			if(node == 0x00) return 0x00; // not available for now
		}
		write_node_data(node , 1 , size , 0);
		// Seperate node and make space
		if(total_node_size >= size) {
			// Separate node to prevent internal fragmentation(Allow residual unused area usable)
			separated_node = (struct Node *)(((max_t)node+sizeof(struct Node))+(size));
			if(separated_node >= node->next) return 0x00; // Separated bigger 
			write_node_data(separated_node , 0 , total_node_size-size-sizeof(struct Node) , 0 , (max_t)node->next , (max_t)node);
		}
	}
	currently_using_mem += size+sizeof(struct Node);
	// Return the actual available address : Node address + size of the node structure
	maximum_node_addr = MAX((max_t)maximum_node_addr , (max_t)node);
	return (((max_t)node+sizeof(struct Node)));			// Actual address that is going to be used : 
																		// Address after area of node
}

/// @brief Deallocate physical kernel memory
/// @param address Address of memory chunk
bool memory::NodesManager::free(max_t address) {
	bool merged = false;
	max_t total_merging_size = 0;
	struct Node *node_ptr;
	struct Node *current_node;
	struct Node *node_next;
	struct Node *node_prev;
	
	if(address < mem_start_address||address > mem_end_address) return false;
	struct Node *node = (struct Node *)(((max_t)address)-sizeof(struct Node));  // address of Node : address - Size of the node structure
	if((node->occupied == 0)||(node->signature != MEMMANAGER_SIGNATURE)) {			// If Node is not using, or not present, print error and leave.
		return false;
	}
	// Allocated Size : Location of the next node - Location of current node
	// If next node is usable, and present, the node can be merged.
	currently_using_mem -= node->size+sizeof(struct Node);
	if((node->next != 0x00) && (node->next->occupied == 0) && (node->next->signature == MEMMANAGER_SIGNATURE)) {
		// printf("Next mergable\n");
		merged = true;
		current_node = node;									// current_node : Saves the current node for later
		node_ptr = node;									// Save the current node, and move to next node
		while((node_ptr->next != 0x00) && (node_ptr->next->occupied == 0) && (node_ptr->signature == MEMMANAGER_SIGNATURE)) {
			if(node_ptr->next == 0x00) break;
			node_ptr = (struct Node *)node_ptr->next; 				// Go to the next node and keep search
		}
		/*
		printf("Merging Node(Next) : 0x%X~0x%X\n" , current_node , node_ptr);
		printf("Total Merging size : %d\n" , (((max_t)node_ptr->next)-((max_t)current_node)-sizeof(struct Node)));
		*/
		// Done erasing : Modify the next node location to the end of the node(It's going to be using node).
		write_node_data(((struct Node *)current_node) , 0 , (((max_t)node_ptr->next)-((max_t)current_node)-sizeof(struct Node)) , 0 , (max_t)node_ptr->next); // Free the node
	}
	// If the previous node is usable, and present, the node can be merged.
	// (Why are we merging and seperating the segment? Because, it can reduce the external fragmentation)
	if((node->previous != 0x00) && (node->previous->occupied == 0) && (node->previous->signature == MEMMANAGER_SIGNATURE)) {
		// printf("Previous mergable\n");
		merged = true;
		current_node = node;				// current_node : Saves the current node for later
		node_ptr = node;
		while((node_ptr->previous != 0x00) && (node_ptr->previous->occupied == 0) && (node_ptr->signature == MEMMANAGER_SIGNATURE)) {
			// Search the nodes that is available for merging, and erase all usable node to make a free space 
			// -> until we find already using node, or not present node.
			if(node_ptr->previous == 0x00) break;
			node_ptr = (struct Node *)node_ptr->previous; 	    // Head to previous node 
		}
		/*
		printf("Merging Node(Prev) : 0x%X~0x%X\n" , current_node , node_ptr);
		printf("current_node->next : 0x%X\n" , current_node->next);
		printf("Size : %d\n" , (((max_t)current_node->next)-((max_t)node_ptr)-sizeof(struct Node)));
		*/
		write_node_data(node_ptr , 0 , (((max_t)current_node->next)-((max_t)node_ptr)-sizeof(struct Node)) , 0 , (max_t)current_node->next);
	}
	if(merged == false) {
		// printf("No merge\n");
		node->occupied = 0;
		if(node->next == 0x00) {
			// printf("No next free\n");
			node->previous->next = 0x00;
			memset(node , 0 , sizeof(struct Node));
		}
	}
	// If the first node is usable, and there is no next nodes, then the node will be removed.
	// But, if the first node is being used, or there is next nodes, then the node is not going to be removed.
	node = node_start;
	if((node->occupied == 0) && (((struct Node *)node->next) == 0x00)) { // If it sooths the condition,
		memset(node , 0 , sizeof(struct Node));		  // Erase the node(Set everything to 0)
	}
	return true;
}


/// @brief Search the node that is bigger than the given size from the argument
/// @param size "Least minimum" size of desired node
/// @return Location of the desired node
struct memory::Node *memory::NodesManager::search_first_fit(max_t size) {
	struct Node *node;
	node = node_start;
	while(node->signature == MEMMANAGER_SIGNATURE) {
		if((node->occupied == 0) && (node->size >= size) && (node->size-size > sizeof(struct Node))) {
			// debug::out::printf("Free Node Found : At 0x%X, Size : %d, %d\n" , Node , (((max_t)Node->next)-(max_t)Node-sizeof(struct Node)) , Node->Size);
			return node;
		}
		node = node->next;
	}
	return 0; // No node available, need to create new node
}

/// @brief Searches node that is already aligned
/// @param Size Size of the node
/// @param Alignment Option of alignment
/// @return Location of the node
struct memory::Node *memory::NodesManager::search_aligned(max_t size , max_t alignment) {
	struct Node *node;
	max_t aligned_addr;
	node = node_start;
	while(node->signature == MEMMANAGER_SIGNATURE) { // going forward until we meet invalid node
		if(node->occupied == 0) { // If node is usable
			// Check whether this node is aligned, or if aligned in future, fits the required size.
			aligned_addr = align_address(((max_t)node)+sizeof(struct Node) , alignment);
			// Get the aligned address of the node
			if(((aligned_addr+size+sizeof(struct Node)) <= (max_t)node->next)) { // If address of aligned node is above the region of the node -> Skip.
				return (struct Node *)(aligned_addr-sizeof(struct Node));
			}
		}
		node = (struct Node *)node->next; // forward
	}
	return 0x00; // No node available, need to create new node
}

/// @brief Search the location for new node
/// @param prev_node Recipient variable for the location of previous node of new node
/// @return Location of new node, and location of previous node of That node(prev_node)
struct memory::Node *memory::NodesManager::search_new_node_location(max_t *prev_node) {
	struct Node *node;
	if(this->node_start->occupied == 0) {
		// If current address is start of the memory,
		*prev_node = 0x00;
		return this->node_start;  // return the start address.
	}
	// If there is no freed address -> Use current_node
	// If there is freed address 	-> Use last_freed_node
	node = (struct Node *)maximum_node_addr;
	while((node->next != 0x00) && (node->signature != MEMMANAGER_SIGNATURE)) {	// Go to the last node
		node = node->next;
	}
	*prev_node = (max_t)node;
	// Return the location of the last node.
	return (struct Node *)(((max_t)node)+(sizeof(struct Node))+node->size);
}

/// @brief Create new node and link with already existing nodes
/// @param size Size of the new node
/// @param alignment Alignment of address of node
/// @return Location of created node
struct memory::Node *memory::NodesManager::create_new_node(max_t size , max_t alignment) {
	memory::NodesManager *node_mgr = (memory::NodesManager *)/*MEMORYMANAGEMENT_MEMORY_STARTADDRESS*/0x00;
	max_t prev_node = 0x00;
	// Create new node
	struct Node *node = search_new_node_location(&(prev_node));
	// Align
	node = align(node , alignment , prev_node);
	// If not aligned, and there is no node in entire allocation system
	// register to node_start
	if((alignment != 0) && (node_mgr->node_start->occupied == 0)) {
		node_mgr->node_start = node;
		// debug::out::printf("Adjusting node from alignment\n");
	}
	// Next node : Offset + Size of the node + Size of the node structure
	write_node_data(node , 1 , size , alignment , 0x00 , prev_node);
	return node;
}

/// @brief Write data to the node
/// @param node Target node
/// @param occupied occupied flag, 1 = node is occupied
/// @param size size of the node
/// @param alignment alignment byte
/// @param next_node next node pointer
/// @param previous_node previous node pointer
void memory::NodesManager::write_node_data(struct Node *node , unsigned char occupied , max_t size , max_t alignment , max_t next_node , max_t previous_node) {
	node->occupied = occupied;
	if(previous_node != INVALID) { // auto
		node->previous = (struct Node *)previous_node;
	}
	if(next_node != 0xFFFFFFFFFFFFFFFF) { // auto
		node->next = (struct Node *)next_node;
	}
	if(size != 0x00) {
		node->size = size;
	}
	if(node == this->node_start) {					// For the first node, the previous node shouldn't be exist.
		node->previous = 0;												// Set PreviousNode to zero
	}
	if(node->previous != 0x00) {
		node->previous->next = node;
	}
	if(node->next != 0x00) {
		node->next->previous = node;
	}
	node->signature = MEMMANAGER_SIGNATURE;   // Write a signature to mark that it's a valid node
	node->aligned = (alignment != 0);
}

struct memory::Node *memory::NodesManager::align(struct memory::Node *node , max_t alignment , max_t prev_node) {
	max_t aligned = 0;   // PreviousNodeAddress : Previous Node Address before aligning to 4K
	max_t original = (max_t)node;
	if(alignment == 0) return node; // no need to align
	aligned = align_address((((max_t)node)+sizeof(struct Node)) , alignment);  // Get the aligned address
	((struct Node *)(aligned-sizeof(struct Node)))->previous = node->previous; // Write previous node information to new aligned node
	
	node = (struct Node *)(aligned-sizeof(struct Node));							      // Relocate node to aligned address
	if(original != (max_t)this->node_start) {
		node->previous = (struct Node *)prev_node;
		node->previous->next = (struct Node *)(aligned-sizeof(struct Node));	  // Rewrite new node information(new aligned one)
	}
	else {
		node->previous = 0x00;
	}
	return node;
}

max_t memory::align_address(max_t address , max_t alignment) {
	if(alignment == 0) {
		return address;
	}
	return (((max_t)(address/alignment))+1)*alignment;
}