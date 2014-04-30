#include "BTreeNode.h"

using namespace std;

BTLeafNode::BTLeafNode()
{
    keyCount = 0;
    memset(buffer,0,PageFile::PAGE_SIZE);
    keys = (int *)(buffer + sizeof(KeyType));
}
/*
 * Read the content of the node from the page pid in the PageFile pf.
 * @param pid[IN] the PageId to read
 * @param pf[IN] PageFile to read from
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::read(PageId pid, const PageFile& pf)
{  
    RC rc;
    // read the page containing the leaf node
    if ((rc = pf.read(pid, buffer)) < 0) return rc;
      
    // the second four bytes of a page contains # keys in the page
    memcpy(&keyCount, buffer, sizeof(int));
    return 0; 
}
    
/*
 * Write the content of the node to the page pid in the PageFile pf.
 * @param pid[IN] the PageId to write to
 * @param pf[IN] PageFile to write to
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::write(PageId pid, PageFile& pf)
{ 
    RC rc;
    // write the page to the disk
    memcpy(buffer, &keyCount, sizeof(int));
    if ((rc = pf.write(pid, buffer)) < 0) return rc;
      
    return 0; 
}

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTLeafNode::getKeyCount()
{
    return keyCount;    
}

/*
 * Insert a (key, rid) pair to the node.
 * @param key[IN] the key to insert
 * @param rid[IN] the RecordId to insert
 * @return 0 if successful. Return an error code if the node is full.
 */
RC BTLeafNode::insert(KeyType key, const RecordId& rid)
{ return 0; }

/*
 * Insert the (key, rid) pair to the node
 * and split the node half and half with sibling.
 * The first key of the sibling node is returned in siblingKey.
 * @param key[IN] the key to insert.
 * @param rid[IN] the RecordId to insert.
 * @param sibling[IN] the sibling node to split with. This node MUST be EMPTY when this function is called.
 * @param siblingKey[OUT] the first key in the sibling node after split.
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::insertAndSplit(KeyType key, const RecordId& rid, 
                              BTLeafNode& sibling, int& siblingKey)
{ return 0; }

/*
 * Find the entry whose key value is larger than or equal to searchKey
 * and output the eid (entry number) whose key value >= searchKey.
 * Remeber that all keys inside a B+tree node should be kept sorted.
 * @param searchKey[IN] the key to search for
 * @param eid[OUT] the entry number that contains a key larger than or equalty to searchKey
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::locate(int searchKey, int& eid)
{ return 0; }

/*
 * Read the (key, rid) pair from the eid entry.
 * @param eid[IN] the entry number to read the (key, rid) pair from
 * @param key[OUT] the key from the entry
 * @param rid[OUT] the RecordId from the entry
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::readEntry(int eid, int& key, RecordId& rid)
{ return 0; }

/*
 * Return the pid of the next slibling node.
 * @return the PageId of the next sibling node 
 */
PageId BTLeafNode::getNextNodePtr()
{ return 0; }

/*
 * Set the pid of the next slibling node.
 * @param pid[IN] the PageId of the next sibling node 
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::setNextNodePtr(PageId pid)
{ return 0; }




BTNonLeafNode::BTNonLeafNode()
{
    keyCount = 0;
    memset(buffer,0,PageFile::PAGE_SIZE);
    keys = (KeyType *)(buffer + sizeof(int));
    pids = (PageId  *)(keys   + KEYS_PER_PAGE);
}
/*
 * Read the content of the node from the page pid in the PageFile pf.
 * @param pid[IN] the PageId to read
 * @param pf[IN] PageFile to read from
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::read(PageId pid, const PageFile& pf)
{ 
    RC rc;
    if( (rc = pf.read(pid,buffer))<0 ) return rc;
    memcpy(&keyCount, buffer, sizeof(int));
    return 0; 
}
    
/*
 * Write the content of the node to the page pid in the PageFile pf.
 * @param pid[IN] the PageId to write to
 * @param pf[IN] PageFile to write to
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::write(PageId pid, PageFile& pf)
{ 
    RC rc;
    memcpy(buffer, &keyCount, sizeof(int));
    if( (rc = pf.write(pid,buffer)) < 0 ) return rc;
    return 0;
}

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTNonLeafNode::getKeyCount()
{ return 0; }


/*
 * Insert a (key, pid) pair to the node.
 * @param key[IN] the key to insert
 * @param pid[IN] the PageId to insert
 * @return 0 if successful. Return an error code if the node is full.
 */
RC BTNonLeafNode::insert(KeyType key, PageId pid)
{ 
    return 0; 
}

/*
 * Insert the (key, pid) pair to the node
 * and split the node half and half with sibling.
 * The middle key after the split is returned in midKey.
 * @param key[IN] the key to insert
 * @param pid[IN] the PageId to insert
 * @param sibling[IN] the sibling node to split with. This node MUST be empty when this function is called.
 * @param midKey[OUT] the key in the middle after the split. This key should be inserted to the parent node.
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::insertAndSplit(KeyType key, PageId pid, BTNonLeafNode& sibling, int& midKey)
{ return 0; }

/*
 * Given the searchKey, find the child-node pointer to follow and
 * output it in pid.
 * @param searchKey[IN] the searchKey that is being looked up.
 * @param pid[OUT] the pointer to the child node to follow.
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::locateChildPtr(int searchKey, PageId& pid)
{ return 0; }

/*
 * Initialize the root node with (pid1, key, pid2).
 * @param pid1[IN] the first PageId to insert
 * @param key[IN] the key that should be inserted between the two PageIds
 * @param pid2[IN] the PageId to insert behind the key
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::initializeRoot(PageId pid1, KeyType key, PageId pid2)
{
    int add1 = (char *)keys - ((char *)buffer);
    int add2 = (char *)pids - ((char *)buffer);
    printf("keys[0x%x]:0x%x pids[0x%x]\n",add1,key,add2);
    keys[0] = key;
    pids[0] = pid1;
    pids[1] = pid2;
    keyCount ++; 
    return 0; 
}

/*
char* BTNonLeafNode::slotPtr(int n)
{
  // compute the location of the n'th slot in a page.
  // remember that the first four bytes in a page is used to store
  // # keys in the page and each slot consists of an integer and
  // a pageid
  return (buffer + sizeof(int)) + (sizeof(int)+sizeof(PageId))*n;
}

void BTNonLeafNode::readSlot(int n, int& key, PageId& pageid)
{
  // compute the location of the record
  char *ptr = slotPtr( n );
  // read the key
  memcpy(&key, ptr, sizeof(int));
  // read the pageid
  memcpy(&pageid, ptr+sizeof(int), sizeof(PageId));
}

void BTNonLeafNode::writeSlot(int n, const KeyType key, const PageId pageid)
{
  // compute the location of the record
  char *ptr = slotPtr(page, n);
  // store the key
  memcpy(ptr, &key, sizeof(int));
  // store the value.
  memcpy(ptr+sizeof(int), &pageid, sizeof(PageId));
}

*/





