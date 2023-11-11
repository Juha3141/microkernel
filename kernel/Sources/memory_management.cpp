/**
 * @file memory_management.cpp
 * @brief Core system of memory management
 * 
 * @author Ian Juha Cho
 * contact : ianisnumber2027@gmail.com
 */

#include <memory_management.hpp>
#include <string.hpp>
#include <debug.hpp>

void memory::init(void) {
	word memmap_entry_count = 0;
	max_t total_memory = 0;
	memory_map memmap[64];
	// memmap_entry_count = get_memory_map(memmap);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
// Description : Find the suitable node for required size, and if it requires, seperate the node.    //
// The function allocates segment by this sequence :                                                 //
// 1. Search the suitable node, if there is no suitable node, create one.                            //
// 2. If the size of suitable node, seperate the node                                                //
// Creating Node : Create new node after the last node                                               //
// If there is no node : Create new one                                                              //
// Seperating Sequence :                                                                             //
// 1. Create new node in the existing node(Existing node should be set to required size,             //
//      more specifics on the actual code.)                                                          //
// 2. Set the NextNode value of the newly created node to existing node's NextNode value.            //
// 3. Set the PreviousNode value of the newly created node to address of existing node.              //
// 4. Set the NextNode value of the existing node to address of newly created node                   //
// -> the PreviousNode value of the existing node should not be changed.                             //
//                                                                                                   //
// Examples of the situation :                                                                       //
// 1. There is no suitable segment                 2. There is suitable segment                      //
// +-----------------+                               +-----------------+                             //
// |  Require : 3MB  |                             |  Require : 3MB  |                               //
// +-----------------+                               +-----------------+                             //
// Memory Pool :                                    Memory Pool :                                    //
// +-----+-----+-----+---------------------+       +-----------------+-----------------------+       //
// | 1MB | 1MB | 1MB | ...                   |       |       3MB       | ...                   |     //
// +-----+-----+-----+---------------------+       +-----------------+-----------------------+       //
// -> Solve : Create new node after the last node  -> Solve : Allot the suitable memory              //
// +-----+-----+-----+---------------+-----+       +-----------------+-----------------------+       //
// | 1MB | 1MB | 1MB | Soothed : 3MB | ... |       |  Soothed : 3MB  | ...                   |       //
// +-----+-----+-----+---------------+-----+       +-----------------+-----------------------+       //
//                                                                                                   //
// 3. There is bigger segment                        4. The worst case : External Fragmentation      //
// +-----------------+                               +-----------------+                             //
// |  Require : 3MB  |                               |  Require : 3MB  |                             //
// +-----------------+                               +-----------------+                             //
// Memory Pool :                                    Memory Pool :                                    //
// +-----------------------------+---------+       +-----+-----+-----+-----------+-----+-----+       //
// |              5MB            | ...     |       |1MB U|1MB F|1MB U|   2MB F   |1MB U|1MB U|       //
// +-----------------------------+---------+       +-----+-----+-----+-----------+-----+-----+       //
// -> Solve : Seperate the segment                   -> There is no space to allocate, even there is //
// +-----------------+-----------+---------+       actually available space.                         //
// |  Soothed : 3MB  |    2MB    | ...     |       -> Solve : To optimize the allocation             //
// +-----------------+-----------+---------+                                                         //
//                    ^~~~~~~~~~~ Usable                                                             //
///////////////////////////////////////////////////////////////////////////////////////////////////////

static bool is_aligned_4K(max_t addr) {
	if((addr & 0xFFF) == 0x00) {
		return true;
	}
	return false;
}

static bool is_aligned_8K(max_t addr) {
	if((addr & 0x1FFF) == 0x00) {
		return true;
	}
	return false;
}

/// @brief Allocate physical kernel memory
/// @param size Size to allocate
/// @param alignment Alignment
/// @return 
void *memory::kmem_alloc(max_t size , max_t alignment) {
	default_t i;
	max_t address;
	max_t total_node_size = 0;	// TotalNodeSize : Size of the total node that is going to be used for allocation
	// Load the node_mgr from the local address
	
	NodeManager *node_mgr = (NodeManager *)/*MEMORYMANAGEMENT_MEMORY_STARTADDRESS*/0x00;
	if(size == 0) {
		Debug::printf("Allocation Error #0 : Zero allocated size\n");
		return 0x00;
	}
	struct Node *node = (struct Node *)node_mgr->search_first_fit(size); // Search available node
	struct Node *separated_node;
	if(node->next != 0) {			// If the next node is present, set TotalNodeSize to the size of searched node.
		total_node_size = ((max_t)node->next)-((max_t)node)-sizeof(struct Node);	// TotalNodeSize : size of the current node
	}
	if(node == 0x00) { // If we have to create new node, create new node at the end of the segments
		// Debug::printf("Creating new node\n");
		node = node_mgr->create_new_node(size , alignment);
		if(node == 0x00) {
			Debug::printf("Error! no node can be created anymore\n");
			while(1) {
				;
			}
			return 0x00;
		}
		total_node_size = 0;								// Set the value to 0 so that the <Node Seperation Sequence> can't be executed.	
		// Update this->CurrentAddress, this->CurrentAddress : Next node of lastly created node
		node_mgr->current_node = node;
		// Initialize the next node to remove potential error from garbage memory
	}
	else {	// Using already existing node
		// Debug::printf("Using existing node\n");
		// Debug::printf("NodeLocation : 0x%X\n" , Node);
		if(alignment != 0) { // is it not aligned
			// Search New alignable location
			// Debug::printf("Alignment : %d\n" , Alignment);
			node = (struct Node *)node_mgr->search_aligned(size , alignment);
			// Debug::printf("Found aligned, new one : 0x%X\n" , Node);
			if(node == 0) {
				node = node_mgr->create_new_node(size , alignment);
			}
		}
		node_mgr->write_node_data(node , 1 , size , 0);
		// Seperate node
		if(total_node_size >= size) {
			// Separate node to prevent internal fragmentation(Allow residual unused area usable)
			separated_node = (struct Node *)(((max_t)node+sizeof(struct Node))+(size));
			if(separated_node >= node->next) {
				Debug::printf("Something terrible happend\n");
				// panic
				while(1) {
					;
				}
			}
			node_mgr->write_node_data(separated_node , 0 , total_node_size-size-sizeof(struct Node) , 0 , (max_t)node->next , (max_t)node);
		}
	}
	node_mgr->currently_using_mem += size;
	// Return the actual available address : Node address + size of the node structure
	return ((ptr_t *)((max_t)node+sizeof(struct Node)));			// Actual address that is going to be used : 
																		// Address after area of node
}

/* To-do : Create a allocation optimizing system */

// Description : Find the node, deallocate it, and if it's needed, merge the segments that is linearly usable.
////////////////////////////////////////////////////////////////////////////////////////////////
// For example, the function merges segments in those situations :                            //
// Situation #1 :                              Situation #2 :                                 //
//                                                                                            //
// Node to deallocate                                                 Node to deallocate      //
//  VVVVVVVVVVVVVVV                                                  VVVVVVVVVVVVVVV          //
// +---------------+----------------------+  +----------------------+---------------+         //
// |  Using(1MB)   |      Usable(2MB)     |  |      Usable(2MB)     |  Using(1MB)   |         //
// +---------------+----------------------+  +----------------------+---------------+         //
// <Deallocation Sequence>                   <Deallocation Sequence>                          //
// 1. Go to the next node and check if       1. Go to the previous node and check if          //
//    it's usable(mergable) until we hit     it's usable until we hit not usable node         //
//    not usable node                                                                         //
// 2. Change the target node(= Node to       2. Change the NextNode of the last node          //
//    deallocate)'s NextNode to the node     that we lastly found from searching to           //
//       that we lastly found from searching     target node's next node.                     //
// 3. Change the node's PreviousNode to      3. Set the last node's flag to usable            //
//    target node                                                                             //
// 4. Set the target node's flag to usable                                                    //
// +---------------+----------------------+  +---------------+----------------------+         //
// |  Usable(1MB)  |      Usable(2MB)     |  |      Usable(2MB)     |  Usable(1MB)  |         //
// +---------------+----------------------+  +---------------+----------------------+         //
// This can be merged to :                      This can be also merged to :                  //
// +--------------------------------------+  +--------------------------------------+         //
// |             Usable(3MB)              |  |             Usable(3MB)              |         //
// +--------------------------------------+  +--------------------------------------+         //
////////////////////////////////////////////////////////////////////////////////////////////////


/// @brief Deallocate physical kernel memory
/// @param address Address of memory chunk
void memory::kmem_free(ptr_t *address) {
	bool merged = false;
	max_t total_merging_size = 0;
	struct Node *node_ptr;
	struct Node *current_node;
	struct Node *node_next;
	struct Node *node_prev;
	// Temporal Change : Memory management system doesn't work properly. 
	// node_prev -> often value is corrupted....
	// Load the node_mgr from the local address
	NodeManager *node_mgr = (NodeManager *)/*MEMORYMANAGEMENT_MEMORY_STARTADDRESS*/0x00;
	if((max_t)address < (max_t)node_mgr) {
		Debug::printf("Deallocation Error #0 : Low Memory address , 0x%X\n" , address);
		return;
	}
	struct Node *node = (struct Node *)(((max_t)address)-sizeof(struct Node));  // address of Node : address - Size of the node structure
	if((node->occupied == 0)||(node->signature != MEMMANAGER_SIGNATURE)) {			// If Node is not using, or not present, print error and leave.
		Debug::printf("Deallocation Error #1 : Not Allocated Memory , 0x%X\n" , address);
		return;
	}
	// Allocated Size : Location of the next node - Location of current node
	// If next node is usable, and present, the node can be merged.
	node_mgr->currently_using_mem -= node->size;
	if((node->next != 0x00) && (node->next->occupied == 0) && (node->next->signature == MEMMANAGER_SIGNATURE)) {
		// Debug::printf("Next mergable\n");
		merged = true;
		current_node = node;									// current_node : Saves the current node for later
		node_ptr = node;									// Save the current node, and move to next node
		while((node_ptr->next->occupied == 0) && (node_ptr->signature == MEMMANAGER_SIGNATURE)) {
			if(node_ptr->next == 0x00) {
				break;
			}
			node_ptr = (struct Node *)node_ptr->next; 				// Go to the next node and keep search
		}
		/*
		Debug::printf("Merging Node(Next) : 0x%X~0x%X\n" , current_node , node_ptr);
		Debug::printf("Total Merging size : %d\n" , (((max_t)node_ptr->next)-((max_t)current_node)-sizeof(struct Node)));
		*/
		// Done erasing : Modify the next node location to the end of the node(It's going to be using node).
		node_mgr->write_node_data(((struct Node *)current_node) , 0 , (((max_t)node_ptr->next)-((max_t)current_node)-sizeof(struct Node)) , 0 , (max_t)node_ptr->next); // Free the node
	}
	// If the previous node is usable, and present, the node can be merged.
	// (Why are we merging and seperating the segment? Because, it can reduce the external fragmentation)
	if((node->previous != 0x00) && (node->previous->occupied == 0) && (node->previous->signature == MEMMANAGER_SIGNATURE)) {
		// Debug::printf("Previous mergable\n");
		merged = true;
		current_node = node;				// current_node : Saves the current node for later
		node_ptr = node;
		while((node_ptr->previous->occupied == 0) && (node_ptr->signature == MEMMANAGER_SIGNATURE)) {
			// Search the nodes that is available for merging, and erase all usable node to make a free space 
			// -> until we find already using node, or not present node.
			if(node_ptr->previous == 0x00) {
				break;
			}
			node_ptr = (struct Node *)node_ptr->previous; 	    // Head to previous node 
		}
		/*
		Debug::printf("Merging Node(Prev) : 0x%X~0x%X\n" , current_node , node_ptr);
		Debug::printf("current_node->next : 0x%X\n" , current_node->next);
		Debug::printf("Size : %d\n" , (((max_t)current_node->next)-((max_t)node_ptr)-sizeof(struct Node)));
		*/
		node_mgr->write_node_data(node_ptr , 0 , (((max_t)current_node->next)-((max_t)node_ptr)-sizeof(struct Node)) , 0 , (max_t)current_node->next);
		node_mgr->last_freed_node = node_ptr;
		/*
		next_node = node->next; // Next node of original node
		node_prev = node->previous->previous;
		node_mgr->write_node_data(node->previous , 0 , ((max_t)Node)-(max_t)node->previous-sizeof(struct Node) , (max_t)next_node , (max_t)node_prev);
		*/
	}
	if(merged == false) {
		// Debug::printf("No merge\n");
		node->occupied = 0;
		node_mgr->last_freed_node = node;
		if(node->next == 0x00) {
			// Debug::printf("No next free\n");
			node->previous->next = 0x00;
			memset(node , 0 , sizeof(struct Node));
		}
	}
	// If the first node is usable, and there is no next nodes, then the node will be removed.
	// But, if the first node is being used, or there is next nodes, then the node is not going to be removed.
	node = node_mgr->node_start;
	if((node->occupied == 0) && (((struct Node *)node->next) == 0x00)) { // If it sooths the condition,
		memset(node , 0 , sizeof(struct Node));		  // Erase the node(Set everything to 0)
		node_mgr->current_node = node_mgr->node_start;	// Reset the address so that
		node_mgr->last_freed_node = node_mgr->node_start;  // Next allocation will be started at StartAddress
	}
}

/// @brief Initializes the variables of class
/// @param start_address start address of memory
/// @param total_usable_mem size of total usable memory
/// @param memmap_count number of memory maps
/// @param memmap memory maps structure
void memory::NodeManager::init(max_t start_address , max_t total_usable_mem , int memmap_count , memory_map *memmap) {
	int i;
	this->node_start = (struct Node *)start_address;   // StartAddress 	       : The location of the memory pool
	this->current_node = (struct Node *)start_address; // CurrentAddress       : The location of current position
	this->last_freed_node = 0;						   // LastFreedAddress     : The location of lastly freed segment, 0 = There's yet no freed segment
	this->total_usable_memory = total_usable_mem;	   // TotalUsableMemory    : Total available memory to use(Size of the memory pool)
	this->currently_using_mem = 0;					   // CurrentlyUsingMemory : Currently using memory size
	/*
	To-do : Querying memory
	for(i = this->unusable_memories_count = 0; i < memmap_count; i++) {
		if(E820[i].Type != ) { // If memory is not usable, put it into unusable memories list
			memcpy(&(unusable_memories[UnusableMemoryEntryCount]) , &(E820[i]) , sizeof(QuerySystemAddressMap));
			UnusableMemoryEntryCount++;
		}
	}
	*/
}

/// @brief Search the node that is bigger than the given size from the argument
/// @param size "Least minimum" size of desired node
/// @return Location of the desired node
struct memory::Node *memory::NodeManager::search_first_fit(max_t size) {
	struct Node *node;
	node = this->node_start;
	while(node->signature == MEMMANAGER_SIGNATURE) {
		if((node->occupied == 0) && (node->size >= size) && (node->size-size > sizeof(struct Node))) {
			// Debug::printf("Free Node Found : At 0x%X, Size : %d, %d\n" , Node , (((max_t)Node->next)-(max_t)Node-sizeof(struct Node)) , Node->Size);
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
struct memory::Node *memory::NodeManager::search_aligned(max_t size , max_t alignment) {
	struct Node *node;
	max_t aligned_addr;
	node = (struct Node *)((this->last_freed_node == 0) ? this->current_node : this->last_freed_node);
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
struct memory::Node *memory::NodeManager::search_new_node_location(max_t *prev_node) {
	struct Node *node;
	if(this->node_start->occupied == 0) {
		// If current address is start of the memory,
		*prev_node = 0x00;
		return this->node_start;  // return the start address.
	}
	// If there is no freed address -> Use current_node
	// If there is freed address 	-> Use last_freed_node
	node = this->current_node;
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
struct memory::Node *memory::NodeManager::create_new_node(max_t size , max_t alignment) {
	memory::NodeManager *node_mgr = (memory::NodeManager *)/*MEMORYMANAGEMENT_MEMORY_STARTADDRESS*/0x00;
	max_t prev_node = 0x00;
	// Create new node
	struct Node *node = search_new_node_location(&(prev_node));
	// Align
	node = align(node , alignment , prev_node);
	// If not aligned, and there is no node in entire allocation system
	// register to node_start
	if((alignment != 0) && (node_mgr->node_start->occupied == 0)) {
		node_mgr->node_start = node;
		node_mgr->current_node = node;
		// Debug::printf("Adjusting node from alignment\n");
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
void memory::NodeManager::write_node_data(struct Node *node , unsigned char occupied , max_t size , max_t alignment , max_t next_node , max_t previous_node) {
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

max_t memory::align_address(max_t address , max_t alignment) {
	if(alignment == 0) {
		return address;
	}
	return (((max_t)(address/alignment))+1)*alignment;
}

struct memory::Node *memory::NodeManager::align(struct memory::Node *node , max_t alignment , max_t prev_node) {
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