#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <set>
#include <unordered_set>
#include <vector>
#include <unordered_map>
#include <ctime>
#include "Struct_type.h"
#include "RegexToDFA.h"
#include "TimerClock.h"

using namespace std;

FinalDFA dfa;
TreeNode *a;
int numNode = 0;
int labelNum = 0;
int resultNumber = 0;
vector<vector<edge>> graph;
vector<vector<edge>> Back_graph;
TimerClock TC;
vector<vector<int>> B;
vector<int> B_length;
vector<unordered_map<int,int>> node_to_state;

vector<string> Split(const string& s, const char delim = '\t') {
	vector<string> elements;
	stringstream ss(s);
	string item;
	while (getline(ss, item, delim)) {
		elements.push_back(item);
	}
	return elements;
}

void DeleteGraph(int src,int label,int dst){
    for(auto it = graph[src].begin();it != graph[src].end();){
        if(it->label == label && it->toNode == dst){
            graph[src].erase(it);
            break;
        }else{
            it++;
        }
    }
    for(auto it = Back_graph[dst].begin();it != Back_graph[dst].end();){
        if(it->label == label && it->toNode == src){
            Back_graph[dst].erase(it);
            break;
        }else{
            it++;
        }
    }
}

void RecoverGraph(int src,int label,int dst){
    edge t;
    bool f = false;
    bool b = false;
    for(auto it = graph[src].begin();it != graph[src].end();it++){
        if(it->label == label && it->toNode == dst){
            f = true;
        }
    }
    if(!f){
        t.label = label;
        t.toNode = dst;
        graph[src].push_back(t);
    }
    for(auto it = Back_graph[dst].begin();it != Back_graph[dst].end();it++){
        if(it->label == label && it->toNode == src){
            b = true;
        }
    }
    if(!b){
        t.label = label;
        t.toNode = src;
        Back_graph[dst].push_back(t);
    }
}

void Add_path(TreeNode *temp,int src,int place,int dst){
    if(!temp->nextnode.count(src)){
        temp->nextnode[src][-1] = new TreeNode(src);
        temp->nextnode[src][-1]->start = true;
    }
    temp = temp->nextnode[src][-1];
    for(int j = 1;j < B[place].size() - 1;){
        if(!temp->nextnode.count(B[place][j+1])){
            temp->nextnode[B[place][j + 1]][B[place][j]] = new TreeNode(B[place][j + 1],B[place][j]);
        }else{
            if(!temp->nextnode[B[place][j + 1]].count(B[place][j])){
                temp->nextnode[B[place][j + 1]][B[place][j]] = new TreeNode(B[place][j + 1],B[place][j]);
            }
        }
        if(B[place][j + 1] == dst){
            temp->nextnode[B[place][j + 1]][B[place][j]]->finish = true;
        }
        temp = temp->nextnode[B[place][j + 1]][B[place][j]];
        j = j + 2;
    }
    B.erase(B.begin() + place);
    B_length.erase(B_length.begin() + place);
    node_to_state.erase(node_to_state.begin() + place);
}    

bool find_shorest_path(int src,int start_state,int dst){
    //init vector;
    vector<pair<int,int>> Q;
    vector<vector<pair<int,int>>> BFS_record;
    vector<vector<bool>> BFS_check;
    vector<vector<int>> BFS_label;
    Q.clear();
    BFS_record.clear();
    BFS_check.clear();
    BFS_label.clear();
    BFS_record.resize(numNode);
    for(int i = 0; i < numNode;i++){
        BFS_record[i].resize(dfa.stateNum);
    }
    BFS_check.resize(numNode);
    for(int i = 0; i < numNode;i++){
        BFS_check[i].resize(dfa.stateNum,false);
    }
    BFS_label.resize(numNode);
    for(int i = 0; i < numNode;i++){
        BFS_label[i].resize(dfa.stateNum);
    }
    Q.push_back({src,start_state});
    BFS_check[src][start_state] = true;

    //begin;
    while(!Q.empty()){
        int node = Q.begin()->first;
        int state = Q.begin()->second;
        Q.erase(Q.begin());
        for(auto it = graph[node].begin();it != graph[node].end();it++){
            int label = it->label;
            if(dfa.labelToState[state].count(label)){
                int nextnode = it->toNode;
                int nextstate = dfa.labelToState[state][label];
                if(!BFS_check[nextnode][nextstate]){
                    BFS_check[nextnode][nextstate] = true;
                    Q.push_back({nextnode,nextstate});
                    BFS_record[nextnode][nextstate] = {node,state};
                    BFS_label[nextnode][nextstate] = label;
                }
                if(nextnode == dst && dfa.endStates.count(nextstate)){
                    //find a path;
                    //cout << "find a path!" << endl;
                    vector<int> r;
                    unordered_map<int,int> nodestate;
                    int common_node = nextnode;
                    int common_state = nextstate;
                    while(common_node != src){
                        r.insert(r.begin(),common_node);
                        r.insert(r.begin(),BFS_label[common_node][common_state]);
                        pair<int,int> temp;
                        temp = BFS_record[common_node][common_state];
                        common_node = temp.first;
                        common_state = temp.second;
                        nodestate[common_node] = common_state;
                    }
                    r.insert(r.begin(),src);
                    B.push_back(r);
                    B_length.push_back(r.size());
                    node_to_state.push_back(nodestate);
                    return true;
                }
            }
        }
    }
    return false;
}

bool Type_Yen(int src,int dst){
    a = new TreeNode(-1,-1);
    //cout << "making pre_tree" << endl;
    find_shorest_path(src,dfa.startState,dst);
    resultNumber++;
    //cout << resultNumber << endl;
    vector<tuple<int,int,int>> e;
    while(!B.empty()){
        //find shorest path;
        int min = B_length[0];
        int place = 0;
        for(int i = 0; i < B_length.size();i++){
            if(B_length[i] < min){
                min = B_length[i];
                place = i;
            }
        }
        vector<int> p = B[place];
        unordered_map<int,int> record_state = node_to_state[place];
        //add the shorest path;
        Add_path(a,src,place,dst);
        if(TC.getTimerSecond() > 600 || resultNumber > 1000){
            return false;
        }
        //begin Yen
        for(int i = 0;i < p.size()-2;){
            int s = p[i];
            int l = p[i+1];
            int t = p[i+2];
            DeleteGraph(s,l,t);
            bool de = false;
            TreeNode *temp = a;
            if(i == 0){
                for(auto it = temp->nextnode[src][-1]->nextnode.begin();it != temp->nextnode[src][-1]->nextnode.end();it++){
                    int nextnode = it->first;
                    for(auto iter = it->second.begin();iter != it->second.end();iter++){
                        int label = iter->first;
                        e.push_back({src,label,nextnode});
                        DeleteGraph(src,label,nextnode);
                        de = true;
                    }
                }
            }else{
                int j = 1;
                int k = 0;
                temp = temp->nextnode[src][-1];
                while(j < i){
                    if(temp->nextnode.count(p[j+1])){
                        if(temp->nextnode[p[j+1]].count(p[j])){
                            temp = temp->nextnode[p[j+1]][p[j]];
                            k = j + 1;
                        }
                        j = j + 2;
                    }
                }
                if(k == i){
                    for(auto it = temp->nextnode.begin();it != temp->nextnode.end();it++){
                        int nextnode = it->first;
                        for(auto iter = it->second.begin();iter != it->second.end();iter++){
                            int label = iter->first;
                            e.push_back({temp->value,label,nextnode});
                            DeleteGraph(temp->value,label,nextnode);
                            de = true;
                        }
                    }
                }
            }
            int state = record_state[s];
            if(find_shorest_path(s,state,dst)){
                resultNumber++;
                int size = B.size()-1;
                for(int g = i-1;g >= 0;g--){
                    B[size].insert(B[size].begin(),p[g]);
                }
                B_length[size] = B[size].size();
            }
            RecoverGraph(s,l,t);
            if(de){
                for(auto i : e){
                    RecoverGraph(get<0>(i),get<1>(i),get<2>(i));
                }
                e.clear();
            }
            i = i+2;
            if(TC.getTimerSecond() > 600 || resultNumber > 1000){
                return false;
            }
        }
    }
}

void TypeSearch(int src,int dst,string regex){
    RegexToDFA d;
    d.update(regex);
    dfa = d.dfa;
    Type_Yen(src,dst);
    a->~TreeNode();
}

void ReadData(string edgedata){
    ifstream infile(edgedata,ios::in);
    string line;
    getline(infile,line);
    istringstream ss(line);
    ss >> numNode;
    ss >> labelNum;
    graph.resize(numNode + 1);
    Back_graph.resize(numNode + 1);
    while(getline(infile, line)){
        istringstream ss(line);
        int src,dst,label;
        ss >> src;
        ss >> dst;
        ss >> label;
        //cout << src << " " << dst << " " << label << endl;
        if(src != dst){
            edge temp;
            temp.toNode = dst;
            temp.label = label;
            graph[src].push_back(temp); 
            edge temp1;
            temp1.toNode = src;
            temp1.label = label;
            Back_graph[dst].push_back(temp1);
        }
    }
}

void clear_vector(){
    resultNumber = 0;
    B.clear();
    B_length.clear();
    node_to_state.clear();
}

int main(int argc, char* argv[]){

    string edgeFile = argv[1];
    string queryFile = edgeFile + "_Type_true.query";
    string resultFile = edgeFile + "_Type_Yen_number.result";

    // string queryFile = argv[2];
    // string resultFile = argv[3];
    // string TimeFile = argv[3];

    ReadData(edgeFile);//read data;
    cout<<"node number: " << numNode << endl;
    cout<<"label number: " << labelNum << endl;

    ofstream s;
    s.open(resultFile.c_str());

    ifstream query;
    //query form : start end regex;
    query.open(queryFile.c_str());
    string line;
    vector<string> temp;
    int i = 0;
    while(getline(query,line)){
        temp = Split(line,' ');
        long long src = stoll(temp[0]);
        long long dst = stoll(temp[1]);
        string regex = temp[2];
        i++;
        //cout << i << " " << src << " " << dst << " " << regex << endl;
        TC.update();
        TypeSearch(src,dst,regex);
        double time = TC.getTimerMilliSec();
        //outputResult(resultFile,i,src);
        cout << i << " " << resultNumber << " " << time << endl;
        s << i << " " << resultNumber << " " << time << endl;
        //ResultTime(resultFile,time,i,bfs_time);
        clear_vector();
    }
    query.close();
    s.close();
    return 0;
}