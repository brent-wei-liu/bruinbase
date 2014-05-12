/*
 * Copyright (C) 2014 by The Regents of the Floridar International University
 * Redistribution of this file is permitted under the terms of the GNU
 * Public License (GPL).
 *
 * @author Wei Liu <wliu015@cs.fiu.edu>
 * @date 5/12/2014
 */
 
#include "BTreeIndex.h"
#include <queue>
using namespace std;

static PageId getRootPid(const char* page);
static void setRootPid(char* page, PageId pid);

static int getTreeHeight(const char* page);
static void setTreeHeight(char* page, int height);

/*
 * BTreeIndex constructor
 */
BTreeIndex::BTreeIndex()
{
    newPid = 0;
    rootPid = -1;
    treeHeight = -1;
}

/*
 * Open the index file in read or write mode.
 * Under 'w' mode, the index file should be created if it does not exist.
 * @param indexname[IN] the name of the index file
 * @param mode[IN] 'r' for read, 'w' for write
 * @return error code. 0 if no error
 */
RC BTreeIndex::open(const string& indexname, char mode)
{
  RC   rc;
  char page[PageFile::PAGE_SIZE];

  // open the page file
  if ((rc = pf.open(indexname, mode)) < 0) return rc;

  //
  // in the rest of this function, we set the rootPid and  treeHeight
  //

  newPid = pf.endPid();
  // if the end pid is zero, the file is empty.
  // set the end record id to (0, 0).
  if (pf.endPid() == 0) {
    rootPid = -1;
    treeHeight = 0;
    return 0;
  }

  if ((rc = pf.read(0, page)) < 0) {
    // an error occurred during page read
    rootPid  = -1;
    treeHeight = 0;
    pf.close();
    return rc;
  }

  // get rootPid and treeHeight in the first page
  rootPid = getRootPid(page);
  treeHeight = getTreeHeight(page);

  return 0;

}

/*
 * Close the index file.
 * @return error code. 0 if no error
 */
RC BTreeIndex::close()
{
  RC rc = 0;
  char page[PageFile::PAGE_SIZE];
  memset(page,0,PageFile::PAGE_SIZE);
  sprintf(page,"%d %d\n",rootPid,treeHeight);
  //setRootPid(page, rootPid);
  //setTreeHeight(page, treeHeight);
  if ((rc = pf.write(0, page)) < 0) return rc;

  if ( newPid != pf.endPid() ){
      printf("newPid != pf.endPid() error!\n");
      rc = -1;
  }
  rootPid = 0;
  treeHeight = 0;
  pf.close();
  return rc;
}

/*
 * Find the leaf node that should contain key
 * @param key[IN] the key for the value inserted into the index
 * @param pid[OUT] 
 * @return error code. 0 if no error
 */

RC BTreeIndex::findLeafNode(KeyType key, PageId& pid)
{
    /*
    PageId p = rootPid;
    BTNode  node;
    RC rc;
    int i;
    if(rootPid == -1 || treeHeight<=0){
        printf("error\n");
        return -1;
    }
    for(i=treeHeight; i>0; i--){
        printf("findLeaf read page[%d]\n",p);
        if( (rc = node.read(p,pf)) != 0)  goto ERROR;
        if( (rc = node.locateChildPtr(key, p)) != 0) goto ERROR;
    }
    
    printf("findLeaf return leafNode:[%d]\n",p);
    pid = p;    
    return 0;

ERROR:
    printf("error\n");
    return rc;
    */
    return 0;
}

/*
 * Insert (key, RecordId) pair to the index.
 * @param key[IN] the key for the value inserted into the index
 * @param rid[IN] the RecordId for the record being inserted into the index
 * @return error code. 0 if no error
 */
RC BTreeIndex::insert(KeyType key, const RecordId& rid)
{
  RC   rc;
  PageId pid;
  char page[PageFile::PAGE_SIZE];
  BTNode root;
  //printf("\n***************Insert key:"ANSI_COLOR_RED"%d"ANSI_COLOR_RESET" into Tree ******************\n",key);
  printf("\n**************** Insert key:%d into Tree ******************\n",key);
  if( rootPid == -1){
      BTNode lnode,rnode;
      lnode.isLeaf = rnode.isLeaf = true;
      root.initializeRoot(2,key,3);
      
      rc = root.write(1,pf);
      if(rc != 0) goto ERROR;
      rc = lnode.write(2,pf);
      if(rc != 0) goto ERROR;
      rc = rnode.write(3,pf);
      if(rc != 0) goto ERROR;
      
      rnode.insertNonFull(key, rid, newPid, pf);
      rnode.write(pf);

      rootPid = 1; 
      treeHeight = 1;
      newPid = 4; 
      if(pf.endPid() != 4){
          printf("error: pf.endPid() = %d\n",pf.endPid());
          return -1;
      }
      return 0;
  }
  
  rc = root.read(rootPid, pf);
  if(rc != 0) goto ERROR;
  
  if( root.n == 2*root.getT() - 1){
      printf("New root:%d, height=%d\n",newPid, treeHeight + 1);
      //new root
      BTNode s;
      s.isLeaf = false;
      s.n = 0;
      s.pids[0] = rootPid;
      rootPid = s.pid = newPid;
      newPid++;
      rc = s.splitChild(0,newPid, pf);
      newPid ++;
      if(rc != 0) goto ERROR;
      rc = s.insertNonFull(key, rid, newPid, pf);
      if(rc != 0) goto ERROR;
      rc = s.write(rootPid,pf); 
      if(rc != 0) goto ERROR;
      treeHeight ++;
      
      printTree();
  }else{
      root.insertNonFull(key, rid, newPid, pf);
  }
  
  return 0;
ERROR:
  printf("error\n");
  return -1;
}

/*
 * Find the leaf-node index entry whose key value is larger than or 
 * equal to searchKey, and output the location of the entry in IndexCursor.
 * IndexCursor is a "pointer" to a B+tree leaf-node entry consisting of
 * the PageId of the node and the SlotID of the index entry.
 * Note that, for range queries, we need to scan the B+tree leaf nodes.
 * For example, if the query is "key > 1000", we should scan the leaf
 * nodes starting with the key value 1000. For this reason,
 * it is better to return the location of the leaf node entry 
 * for a given searchKey, instead of returning the RecordId
 * associated with the searchKey directly.
 * Once the location of the index entry is identified and returned 
 * from this function, you should call readForward() to retrieve the
 * actual (key, rid) pair from the index.
 * @param key[IN] the key to find.
 * @param cursor[OUT] the cursor pointing to the first index entry
 *                    with the key value.
 * @return error code. 0 if no error.
 */
RC BTreeIndex::locate(KeyType searchKey, IndexCursor& cursor)
{
    return 0;
}

/*
 * Read the (key, rid) pair at the location specified by the index cursor,
 * and move foward the cursor to the next entry.
 * @param cursor[IN/OUT] the cursor pointing to an leaf-node index entry in the b+tree
 * @param key[OUT] the key stored at the index cursor location.
 * @param rid[OUT] the RecordId stored at the index cursor location.
 * @return error code. 0 if no error
 */
RC BTreeIndex::readForward(IndexCursor& cursor, KeyType& key, RecordId& rid)
{
    return 0;
}

RC BTreeIndex::printTree()
{
    RC rc;
    int i;
    queue<BTNode> q;
    BTNode root;
    if(rootPid == -1) return -1;
    rc = root.read(rootPid, pf);
    if(rc != 0) goto ERROR;
    q.push(root);
    printf("\n\n****************** PRINT TREE **********************\n");
    printf("rootPid:%d pageNum:%d treeHeight:%d\n\n",rootPid,  pf.endPid(), treeHeight);
    for(i=1; i< pf.endPid(); i++){
        BTNode s;
        rc = s.read(i, pf);
        if(rc != 0) goto ERROR;
        s.printNode();
    }
    printf("\n****************** BFS TREE**********************\n");
    while(!q.empty()){
        BTNode u = q.front();
        q.pop();
        if(u.isLeaf){
            printf("----------------->Leaf Node <----------------\n");
            u.printNode();
        }else{
            printf("----------------->Non Leaf Node <----------------\n");
            u.printNode();
            for(int i=0; i<=u.n; i++){
                BTNode v;
                rc = v.read(u.pids[i], pf);
                if(rc != 0) goto ERROR;
                q.push( v );
            }
        }
    }
    printf("\n****************** PRINT TREE END**********************\n");
    return 0;
ERROR:
    printf("printTree error %d\n",rc);
    return rc;
    
}

static PageId getRootPid(const char* page)
{
  PageId rootPid;

  // the first four bytes of a page contains rootPid in the page
  memcpy(&rootPid, page, sizeof(PageId));
  return rootPid;
}


static void setRootPid(char* page, PageId pid)
{
  // the first four bytes of a page contains rootPid in the page
  memcpy(page, &pid, sizeof(PageId));
}

static int getTreeHeight(const char* page)
{
  int height;

  // the second four bytes of a page contains tree height in the page
  memcpy(&height, page, sizeof(int));
  return height;
}


static void setTreeHeight(char* page, int height)
{
  // the second four bytes of a page contains tree height in the page
  memcpy(page, &height, sizeof(int));
}

