/*
 * Copyright (C) 2014 by The Regents of the Floridar International University
 * Redistribution of this file is permitted under the terms of the GNU
 * Public License (GPL).
 *
 * @author Wei Liu <wliu015@cs.fiu.edu>
 * @date 5/12/2014
 */


#include "BTreeNode.h"

using namespace std;
BTNode::BTNode()
{
    n = 0;
    isLeaf = false;
    nextPage = -1;
    pid = -1;
    memset(buffer,0,PageFile::PAGE_SIZE);
    keys = (KeyType *)(buffer + sizeof(bool) + sizeof(int) +sizeof(int));
    rids = (RecordId *)(keys + KEYS_PER_LEAF_PAGE);
    pids = (PageId *)(keys +  KEYS_PER_NONLEAF_PAGE);
}
BTNode::BTNode(const BTNode& n)
{
    this->n = n.n;
    this->isLeaf = n.isLeaf;
    this->nextPage = n.nextPage;
    this->pid = n.pid;
    memcpy(this->buffer, n.buffer, PageFile::PAGE_SIZE);
     keys = (KeyType *)(buffer + sizeof(bool) + sizeof(int) +sizeof(int));
    rids = (RecordId *)(keys + KEYS_PER_LEAF_PAGE);
    pids = (PageId *)(keys +  KEYS_PER_NONLEAF_PAGE);

}
/*
 * Read the content of the node from the page pid in the PageFile pf.
 * @param pid[IN] the PageId to read
 * @param pf[IN] PageFile to read from
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNode::read(PageId p, const PageFile& pf)
{  
    RC rc;
    // read the page containing the leaf node
    if ((rc = pf.read(p, buffer)) < 0) return rc;
    this->pid = p;
    
    // the second four bytes of a page contains # keys in the page
    memcpy(&isLeaf, buffer, sizeof(bool));
    memcpy(&n, buffer+sizeof(bool), sizeof(int));
    memcpy(&nextPage, buffer+sizeof(bool)+sizeof(int), sizeof(PageId));
    return 0; 
}
/*
 * Write the content of the node to the page pid in the PageFile pf.
 * @param pid[IN] the PageId to write to
 * @param pf[IN] PageFile to write to
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNode::write( PageFile& pf)
{ 
    RC rc;
    if(pid == -1 ) return -1;
    // write the page to the disk
    memcpy(buffer, &isLeaf, sizeof(bool));
    memcpy(buffer+sizeof(bool), &n, sizeof(int));
    memcpy(buffer+sizeof(bool)+sizeof(int), &nextPage, sizeof(PageId));
    if ((rc = pf.write(this->pid, buffer)) < 0) return rc;
     
    return 0; 
}
   
/*
 * Write the content of the node to the page pid in the PageFile pf.
 * @param pid[IN] the PageId to write to
 * @param pf[IN] PageFile to write to
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNode::write(PageId p,PageFile& pf)
{ 
    RC rc;
    // write the page to the disk
    if(this->pid != p)  printf("WARNING:pid[%d] != p[%d]\n",pid,p);
    memcpy(buffer, &isLeaf, sizeof(bool));
    memcpy(buffer+sizeof(bool), &n, sizeof(int));
    memcpy(buffer+sizeof(bool)+sizeof(int), &nextPage, sizeof(PageId));
    if ((rc = pf.write(p, buffer)) < 0) return rc;
    this->pid = p;
      
    return 0; 
}

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTNode::getKeyCount()
{
    return n;    
}

/*
 * Insert a (key, rid) pair to the node.
 * @param key[IN] the key to insert
 * @param rid[IN] the RecordId to insert
 * @return 0 if successful. Return an error code if the node is full.
 */
RC BTNode::insertNonFull(KeyType key, const RecordId& rid, int &newPid, PageFile &pf)
{ 
    int i = n - 1;
    RC rc = 0;
    if(isLeaf){
        while( i>=0 && key< keys[i] ){
            keys[i + 1] = keys[i];
            i--;
        }
        keys[i+1] = key;
        rids[i+1].pid = rid.pid;
        rids[i+1].sid = rid.sid;
        
        n++;
        printf("insert pid[%d] : key[%d] -> keys[%d]\n",pid, key, i+1);
        printNode();
        write(pf);
        return 0;
    }else{
        BTNode node;
        printf("insert to non leaf node pid[%d] : key[%d]\n",pid, key);
        printNode();
        while(i>=0 && key < keys[i]) i--;
        i++;
        printf("i:%d\n\n",i);
        rc = node.read(pids[i], pf);
        if(rc != 0) goto ERROR;
        if(node.isLeaf){
            printf("Read Leaf Node page:\n");
        }else{
            printf("Read Non Leaf Node page:\n");
        }
        node.printNode();

        if(node.n == 2*node.getT() - 1){
            splitChild(i, newPid, pf);
            newPid++;
            if( key >= keys[i])  i++; // insert in to new child node
            node.read(pids[i], pf);
        }
        if( (rc = node.insertNonFull(key, rid,newPid,  pf)) != 0) goto ERROR;
    }
    return 0;
ERROR:
    printf("error insertNonFull\n");
    return rc;    
}

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
RC BTNode::splitChild(int i, PageId newPid, PageFile& pf)
{   
    RC rc = 0;
    int j;
    BTNode z; //new node
    BTNode y; //child node
    int t;
    printf("Split Child pid:%d  newPid:%d\n",pids[i],newPid);
    if( this->isLeaf == true ) { rc = -1; goto ERROR; }
    if( (rc = y.read(this->pids[i], pf)) != 0) { rc = -2; goto ERROR; } 
    z.isLeaf = y.isLeaf;
    t = z.getT();
    z.pid = newPid;
    if( z.isLeaf ){
        z.n = t;
        for(j=0; j<=t-1; j++){
            z.keys[j] = y.keys[j+t-1];
            z.rids[j] = y.rids[j+t-1];
        }
        y.n = y.n - t;
        for(j=n; j>=i+1; j--)
            keys[j] = keys[j-1];
        for(j=n+1; j>=i+2; j--)
            pids[j] = pids[j-1];
        keys[i] = z.keys[0];
        pids[i+1] = z.pid;
        n++;
    }else{
        z.n = t - 1;
        for(j=0; j<=t-2; j++){
            z.keys[j] = y.keys[j+t];
        }
        for(j=0; j<=t-1; j++){
            z.pids[j] = y.pids[j+t];
        }

        y.n = y.n - z.n - 1;
        for(j=n; j>=i+1; j--)
            keys[j] = keys[j-1];
        for(j=n+1; j>=i+2; j--)
            pids[j] = pids[j-1];
        keys[i] = y.keys[y.n];
        pids[i+1] = z.pid;
        n++;
    }
    this->printNode();
    y.printNode();
    z.printNode();
    y.write( pf );
    this->write( pf );
    z.write( pf);
    return 0;
ERROR:
    printf("error:%d\n",rc);
    return rc;
}

int BTNode::getT()
{
    int t = -1;
    if(isLeaf )    t = (KEYS_PER_LEAF_PAGE+1)/2;
    else  t = (KEYS_PER_NONLEAF_PAGE+1)/2;
    return t;
}
/*
 * Find the entry whose key value is larger than or equal to searchKey
 * and output the eid (entry number) whose key value >= searchKey.
 * Remeber that all keys inside a B+tree node should be kept sorted.
 * @param searchKey[IN] the key to search for
 * @param eid[OUT] the entry number that contains a key larger than or equalty to searchKey
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNode::locate(int searchKey, int& eid)
{ return 0; }

/*
 * Read the (key, rid) pair from the eid entry.
 * @param eid[IN] the entry number to read the (key, rid) pair from
 * @param key[OUT] the key from the entry
 * @param rid[OUT] the RecordId from the entry
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNode::readEntry(int eid, int& key, RecordId& rid)
{ return 0; }

/*
 * Return the pid of the next slibling node.
 * @return the PageId of the next sibling node 
 */
PageId BTNode::getNextNodePtr()
{ return 0; }

/*
 * Set the pid of the next slibling node.
 * @param pid[IN] the PageId of the next sibling node 
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNode::setNextNodePtr(PageId pid)
{ return 0; }




RC BTNode::initializeRoot(PageId pid1, KeyType key, PageId pid2)
{
    int add1 = (char *)keys - ((char *)buffer);
    int add2 = (char *)pids - ((char *)buffer);
    printf("initializeRoot keys[0x%x]:0x%x pids[0x%x]\n",add1,key,add2);
    keys[0] = key;
    pids[0] = pid1;
    pids[1] = pid2;
    n = 1; 
    return 0; 
}


void BTNode::printNode()
{
    int i;
    if(isLeaf){
        printf("pid[%d] n[%d] Max_n[%d] t[%d]:\n", pid, n, KEYS_PER_LEAF_PAGE, getT());
        for(i=0; i<n; i++){
            printf("position:%d\tkey:%d\n",i, keys[i]);
        }
    }else{
        printf( "pid[%d] n[%d] Max_n[%d] t[%d]:\n", pid, n, KEYS_PER_NONLEAF_PAGE, getT());
        for(i=0; i<n; i++)
            printf("%d ",keys[i]);
        printf("\n");
        for(i=0; i<=n; i++)
            printf("%d ",pids[i]);
    }
    printf("\n");
}



