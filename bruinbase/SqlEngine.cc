/**
 * Copyright (C) 2014 by The Regents of FIU
 * Redistribution of this file is permitted under the terms of the GNU
 * Public License (GPL).
 *
 * @author Wei Liu <wliu015@cs.fiu.edu>
 * @date 5/12/2014
 */

#include <cstdio>
#include <iostream>
#include <fstream>
#include <climits>
#include "Bruinbase.h"
#include "SqlEngine.h"


using namespace std;

// external functions and variables for load file and sql command parsing 
extern FILE* sqlin;
int sqlparse(void);


RC SqlEngine::run(FILE* commandline)
{
  fprintf(stdout, "Bruinbase> ");

  // set the command line input and start parsing user input
  sqlin = commandline;
  sqlparse();  // sqlparse() is defined in SqlParser.tab.c generated from
               // SqlParser.y by bison (bison is GNU equivalent of yacc)

  return 0;
}
RC SqlEngine::selectWithIndex(int attr, const string& table, 
        const vector<SelCond>& cond, 
        const BTreeIndex &idx, 
        const RecordFile &rf)
{
    bool isRangeQuery = true;
    KeyType EQKey;
    //range query
    // if KeyType is not int, we need to change this conversion
    KeyType  minKey = INT_MIN;
    KeyType  maxKey = INT_MAX;
    bool minEQ = true;
    bool maxEQ = true;
    KeyType searchKey, key, k;
    RecordId rid;
    string value;
    IndexCursor cursor;
    RC rc;


    for (unsigned i = 0; i < cond.size(); i++) {
        // compute the difference between the tuple value and the condition value
        k = atoi(cond[i].value);
        if(cond[i].attr == 1){
            // skip the tuple if any condition is not met
            switch (cond[i].comp) {
              case SelCond::EQ:
                isRangeQuery = false;
                // if KeyType is not int, we need to change this conversion
                EQKey = k;
                break;
              case SelCond::GT:
                if(minKey < k) {
                    minKey = k;
                    minEQ = false;
                }
                break;
              case SelCond::LT:
                if(maxKey > k){
                    maxKey = k;
                    maxEQ = false;
                }
                break;
              case SelCond::GE:
                if(minKey < k) {
                    minKey = k;
                    minEQ = true;
                }
                break;
              case SelCond::LE:
                if(maxKey > k){
                    maxKey = k;
                    maxEQ = true;
                }
                break;
            }
        }
    }
    //range qery
    if(isRangeQuery){
        DEBUG('s',"***************  Range Query, Start Key:%d , End Key:%d ****************\n",minKey, maxKey);

        rc = idx.locate(minKey, cursor);
        if(rc != 0) goto EXIT;

        printf("|- KEY -|- VALUE -|\n");
        if(cursor.pid == -1)    goto EXIT;
        rc = idx.readForward(cursor, key, rid);
        if(rc != 0)  goto EXIT;
        if(minKey == key && minEQ){
            // read the tuple
            if ((rc = rf.read(rid, key, value)) < 0) {
              fprintf(stderr, "Error: while reading a tuple from table.\n");
              goto EXIT;
            }
            printf("|  %d\t| %s\n",key, value.c_str());
        }
        rc = idx.readForward(cursor, key, rid);
        if(rc != 0)  goto EXIT;

        while(key < maxKey  ){
            // read the tuple
            if ((rc = rf.read(rid, key, value)) < 0) {
              fprintf(stderr, "Error: while reading a tuple from table.\n");
              break;
            }
            printf("|  %d\t| %s\n",key, value.c_str());
            if( cursor.pid == -1 ) break;
            rc = idx.readForward(cursor, key, rid);
            if(rc != 0)  break;
        }
        if(maxKey == key && maxEQ){
            // read the tuple
            if ((rc = rf.read(rid, key, value)) < 0) {
              fprintf(stderr, "Error: while reading a tuple from table.\n");
              goto EXIT;
            }
            printf("|  %d\t| %s\n",key, value.c_str());
        }

        printf("|-----------------|\n\n");
        DEBUG('s',"************End of Range Query***********\n\n");

    }else{//single query
        searchKey = EQKey;
        DEBUG('s',"***************  Search Key:%d ****************\n",searchKey);
        key = searchKey - 1;

        rc = idx.locate(searchKey,cursor);
        if(rc != 0) goto EXIT;

        printf("|- KEY -|- VALUE -|\n");
        if(cursor.pid == -1){
            printf("Not found key:%d in index\n",searchKey);
        }else{
            rc = idx.readForward(cursor, key, rid);
            if(rc != 0) goto EXIT;

            if(searchKey != key ){
                printf("Not found key:%d in index\n",searchKey);
            }else{
                DEBUG('s',"Found key:%d in index, record id:{%d,%d}\n",searchKey, rid.pid, rid.sid);

                // read the tuple
                if ((rc = rf.read(rid, key, value)) < 0) {
                  fprintf(stderr, "Error: while reading a tuple from table.\n");
                  goto EXIT;
                }
                printf("|  %d\t| %s\n",key, value.c_str());
            }
        }

        printf("|-----------------|\n\n");

    }

    rc = 0;
EXIT:
    return rc;

}
RC SqlEngine::selectWithoutIndex(int attr, const string& table, 
        const vector<SelCond>& cond, 
        const RecordFile &rf)
{
    RecordId   rid;  // record cursor for table scanning
    int    count;
    KeyType    key;
    string value;
    int    diff;
    RC rc;
    
    printf("WITHOUT INDEX\n");
    // scan the table file from the beginning
    rid.pid = rid.sid = 0;
    count = 0;
    while (rid < rf.endRid()) {
        // read the tuple
        if ((rc = rf.read(rid, key, value)) < 0) {
          fprintf(stderr, "Error: while reading a tuple from table %s\n", table.c_str());
          return -1;
        }

        // check the conditions on the tuple
        for (unsigned i = 0; i < cond.size(); i++) {
            // compute the difference between the tuple value and the condition value
            switch (cond[i].attr) {
              case 1:
                diff = key - atoi(cond[i].value);
                break;
              case 2:
                diff = strcmp(value.c_str(), cond[i].value);
                break;
            }

            // skip the tuple if any condition is not met
            switch (cond[i].comp) {
              case SelCond::EQ:
                if (diff != 0) goto next_tuple;
                break;
              case SelCond::NE:
                if (diff == 0) goto next_tuple;
                break;
              case SelCond::GT:
                if (diff <= 0) goto next_tuple;
                break;
              case SelCond::LT:
                if (diff >= 0) goto next_tuple;
                break;
              case SelCond::GE:
                if (diff < 0) goto next_tuple;
                break;
              case SelCond::LE:
                if (diff > 0) goto next_tuple;
                break;
            }
        }

        // the condition is met for the tuple. 
        // increase matching tuple counter
        count++;

        // print the tuple 
        switch (attr) {
            case 1:  // SELECT key
              fprintf(stdout, "%d\n", key);
              break;
            case 2:  // SELECT value
              fprintf(stdout, "%s\n", value.c_str());
              break;
            case 3:  // SELECT *
              fprintf(stdout, "%d '%s'\n", key, value.c_str());
              break;
        }

        // move to the next tuple
        next_tuple:
        ++rid;
    }
    // print matching tuple count if "select count(*)"
    if (attr == 4) {
        fprintf(stdout, "%d\n", count);
    }


}
RC SqlEngine::select(int attr, const string& table, const vector<SelCond>& cond)
{
    RecordFile rf;   // RecordFile containing the table
    BTreeIndex idx;

    RC     rc;
    bool index = true;
    bool hasKeyInCon;
    // open the table file
    if ((rc = rf.open(table + ".tbl", 'r')) < 0) {
        fprintf(stderr, "Error: table %s does not exist\n", table.c_str());
        return rc;
    }
  
    rc = idx.open(table + ".idx", 'r');
    if(rc != 0) {
        cout<<"Do Not Have Index File:"<<table<<".idx"<<endl;
        index = false;
    }
    hasKeyInCon = false;
    for (unsigned i = 0; i < cond.size(); i++) {
        if(cond[i].attr==1) {
            hasKeyInCon = true;
            //break;
        }
        //printf("SelCond attr:%d com:%d value:%s\n",cond[i].attr, cond[i].comp, cond[i].value);
    }
    cout<<endl; 
    if( index && hasKeyInCon ){
        rc = selectWithIndex(attr, table, cond, idx, rf);
    }else{
        rc = selectWithoutIndex(attr, table, cond, rf);
    }
    //rc = 0;

    // close the table file and return
    rf.close();
    idx.close();
    return rc;
}

RC SqlEngine::load(const string& table, const string& loadfile, bool index)
{
    RC rc = 0;
    RecordFile rf;   // RecordFile containing the table
    RecordId rid;
    IndexCursor cursor;
    string value;
    KeyType key, searchKey, startKey, endKey;
    int     bpagecnt, epagecnt;

    cout<< "LOAD "<<table<<" FROM "<<loadfile;
    if(index)   cout<<" WITH INDEX";
    cout<<endl;
    BTreeIndex idx;
    rc = rf.open( table + ".tbl", 'w');
    if(rc < 0){
        cout<<"error:"<<table<<".tbl"<<endl;
        return rc;
    }
    
    if(index){
        rc = idx.open( table + ".idx", 'w'); 
        if(rc<0) {
            cout<<"error:"<<table<<".idx"<<endl;
            return rc;
        }
    }
 
    ifstream readfile(loadfile.c_str() );
    string line;
    while(getline(readfile,line)) {
        string value;
        parseLoadLine(line,key,value);
        //printf("key:%d value:%s\n",key,value.c_str());
        RecordId rid;
        RC rc;
        rc = rf.append(key,value,rid);
        if(rc < 0)  goto LOAD_EXIT;
        
        if(index){   
            rc = idx.insert(key, rid);  
            if(rc < 0) goto LOAD_EXIT;
        }
    }
    if(DebugIsEnabled('i')) idx.printTree();
    
    /*    
    searchKey = 101;
    printf("***************  Search Key:%d ****************\n",searchKey);
    key = searchKey - 1;
    bpagecnt = PageFile::getPageReadCount();

    rc = idx.locate(searchKey,cursor);
    if(rc != 0) goto LOAD_EXIT;

    if(cursor.pid == -1){
        printf("Not found key:%d in index\n",searchKey);
    }else{
        rc = idx.readForward(cursor, key, rid);
        if(rc != 0) goto LOAD_EXIT;

        if(searchKey != key ){
            printf("Not found key:%d in index\n",searchKey);
        }else{
            printf("Found key:%d in index, record id:{%d,%d}\n",searchKey, rid.pid, rid.sid);

            // read the tuple
            if ((rc = rf.read(rid, key, value)) < 0) {
              fprintf(stderr, "Error: while reading a tuple from table.\n");
              goto LOAD_EXIT;
            }
            printf("RECORD key:%d value:%s\n",key, value.c_str());
        }
    }
    epagecnt = PageFile::getPageReadCount();
    printf("*****************End of Search Key:%d , read %d pages *********\n\n",searchKey, epagecnt-bpagecnt);


    startKey = 80;
    endKey = 90;
    printf("***************  Range Query, Start Key:%d , End Key:%d ****************\n",startKey, endKey);
    bpagecnt = PageFile::getPageReadCount();

    rc = idx.locate(startKey, cursor);
    if(rc != 0) goto end_query;
    if(cursor.pid == -1)    goto end_query;
    rc = idx.readForward(cursor, key, rid);
    if(rc != 0)  goto end_query;

    while(key < endKey && cursor.pid != -1 ){
        // read the tuple
        if ((rc = rf.read(rid, key, value)) < 0) {
          fprintf(stderr, "Error: while reading a tuple from table.\n");
          break;
        }
        printf("RECORD key:%d value:%s\n",key, value.c_str());

        rc = idx.readForward(cursor, key, rid);
        if(rc != 0)  break;
    }
end_query:
    epagecnt = PageFile::getPageReadCount();
    printf("************End of Range Query, read %d pages ***********\n\n", epagecnt-bpagecnt);
*/
LOAD_EXIT:    
    idx.close();
    rf.close();
    if(rc != 0) cout<<"error"<<endl;
    return rc;
}

RC SqlEngine::parseLoadLine(const string& line, int& key, string& value)
{
    const char *s;
    char        c;
    string::size_type loc;
    
    // ignore beginning white spaces
    c = *(s = line.c_str());
    while (c == ' ' || c == '\t') { c = *++s; }

    // get the integer key value
    key = atoi(s);

    // look for comma
    s = strchr(s, ',');
    if (s == NULL) { return RC_INVALID_FILE_FORMAT; }

    // ignore white spaces
    do { c = *++s; } while (c == ' ' || c == '\t');
    
    // if there is nothing left, set the value to empty string
    if (c == 0) { 
        value.erase();
        return 0;
    }

    // is the value field delimited by ' or "?
    if (c == '\'' || c == '"') {
        s++;
    } else {
        c = '\n';
    }

    // get the value string
    value.assign(s);
    loc = value.find(c, 0);
    if (loc != string::npos) { value.erase(loc); }

    return 0;
}
