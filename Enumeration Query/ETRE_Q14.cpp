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

FinalDFA Type;
int numNode = 0;
int labelNum = 0;
int resultNumber = 0;

int bfs_count = 0;
int search_count = 0;
int search_count_max = 0;
int conflict_size = 0;
int conflict_size_max = 0;
vector<vector<edge>> graph;
vector<vector<edge>> Back_graph;
vector<unordered_set<int>> labelIn;
vector<unordered_set<int>> labelOut;
unordered_map<long long, int> v_index1;
unordered_map<int, long long> v_index2;//node mapping;
unordered_map<int, int> labelIndex1;
unordered_map<int, int> labelIndex2;//label mapping;
//vector<bool> ArriveBool;//determine whether we explore this node;

//Type progress:
vector<vector<bool> > Block;
vector<vector<unordered_map<int,unordered_set<int> > > >  ConflictSet_type;//form : set[node][state] -> ([node][state])
vector<int> resultNode_type;
vector<int> resultLabel_type;
unordered_set<int> passnode;

//result set;
vector<string> result;
TimerClock TC;
TimerClock Bfs;

//bfs:
//vector<unordered_map<int,bool> > BFS_Arrive_f;
vector<unordered_map<int,bool> > BFS_Arrive_b;
queue<pair<int,int> > Q_F;
queue<pair<int,int> > Q_B;

vector<string> Split(const string& s, const char delim = '\t') {
	vector<string> elements;
	stringstream ss(s);
	string item;
	while (getline(ss, item, delim)) {
		elements.push_back(item);
	}
	return elements;
}

// void BFS_F(int src){
//     pair<int,int> temp;
//     temp.first = src;
//     temp.second = dfa.startState;
//     Q_F.push(temp);
//     BFS_Arrive_f[src][dfa.startState] = true;
//     while(!Q_F.empty()){
//         int node = Q_F.front().first;
//         int state = Q_F.front().second;
//         Q_F.pop();
//         for(auto it = graph[node].begin();it != graph[node].end();it++){
//             int label = it->label;
//             if(dfa.labelToState[state].count(label)){
//                 int nextnode = it->toNode;
//                 int nextstate = dfa.labelToState[state][label];
//                 if(!BFS_Arrive_f[nextnode].count(nextstate)){
//                     temp.first = nextnode;
//                     temp.second = nextstate;
//                     Q_F.push(temp);
//                     BFS_Arrive_f[nextnode][nextstate] = true;
//                 }
//             }
//         }
//     }
// }

void BFS_B(int dst,int end_state){
    pair<int,int> temp;
    temp.first = dst;
    temp.second = end_state;
    Q_B.push(temp);
    BFS_Arrive_b[dst][end_state] = true;
    while(!Q_B.empty()){
        int node = Q_B.front().first;
        int state = Q_B.front().second;
        Q_B.pop();
        for(auto it = Back_graph[node].begin();it != Back_graph[node].end();it++){
            int label = it->label;
            if(Type.Back_labelToState[state].count(label)){
                int nextnode = it->toNode;
                for(auto iter = Type.Back_labelToState[state][label].begin();iter != Type.Back_labelToState[state][label].end();iter++){
                    int nextstate = *iter;
                    if(!BFS_Arrive_b[nextnode].count(nextstate)){
                        bfs_count++;
                        temp.first = nextnode;
                        temp.second = nextstate;
                        Q_B.push(temp);
                        BFS_Arrive_b[nextnode][nextstate] = true;
                    }
                }
            }
        }
    }
}

void BFS(int src,int dst){
    for(auto it = Type.endStates.begin();it != Type.endStates.end();it++){
        BFS_B(dst,*it);
    }
}

bool IFblock(int u,int s,int u1,int s1){
    bool f = false;
    if(ConflictSet_type[u][s].count(u1)){
        if(ConflictSet_type[u][s][u1].count(s1)){
            return true;
        }
    }
    return f;
}

void unblock(int u,int s){
    Block[u][s] = false;
    for(auto it = ConflictSet_type[u][s].begin();it != ConflictSet_type[u][s].end();it++){
        int node = it->first;
        for(auto iter = it->second.begin();iter != it->second.end();iter++){
            int state = *iter;
            if(Block[node][state]){
                Block[node][state] = false;
                unblock(node,state);
                conflict_size = conflict_size - 2;
            }
        }
    }
    ConflictSet_type[u][s].clear();
}

bool Type_path(int node,int dst,int state){
    bool f = false;
    if(TC.getTimerSecond() > 600 || resultNumber > 1000){
        return false;
    }
    if(!BFS_Arrive_b[node][state]){
        Block[node][state] = true;
        return false;
    }

    // for(int i = 0;i < Type.stateNum;i++){
    //     Block[node][state] = true;//block node+state;
    // }

    Block[node][state] = true;
    passnode.insert(node);

    resultNode_type.push_back(node);
    if(!Type.labelToState[state].empty()){
        for(auto it = graph[node].begin();it != graph[node].end();it++){
            int label = it->label;
            if(Type.labelToState[state].count(label)){
                int nextnode = it->toNode;
                int nextstate = Type.labelToState[state][label];
                resultLabel_type.push_back(label);
                search_count++;
                if(search_count > search_count_max){
                    search_count_max = search_count;
                }
                if(nextnode == dst && Type.endStates.count(state)){
                    resultNumber++;
                    // string path = "";
                    // for(int i = 0;i < resultNode_type.size();i++){
                    //     path = path + to_string(v_index2[resultNode_type[i]]) + "---<" + to_string(resultLabel_type[i]) + ">---";
                    // }
                    // path = path + to_string(v_index2[dst]);
                    // result.push_back(path);
                    f = true;
                }
                else if(!Block[nextnode][nextstate] && !passnode.count(nextnode)){
                    if(Type_path(nextnode,dst,nextstate)){
                        f = true;
                    }
                }
                resultLabel_type.pop_back();
                search_count--;
            }
        }
    }
    passnode.erase(node);
    resultNode_type.pop_back();
    if(f){
        //unblock v;
        // for(int i = 0;i < Type.stateNum;i++){
        //     unblock(node,i);
        // }
        unblock(node,state);
    }else{
        for(auto it = graph[node].begin();it != graph[node].end();it++){
            int label = it->label;
            if(Type.labelToState[state].count(label)){
                int nextnode = it->toNode;
                int nextstate = Type.labelToState[state][label];
                if(!IFblock(nextnode,nextstate,node,state) && BFS_Arrive_b[node][state]){
                    ConflictSet_type[nextnode][nextstate][node].insert(state);
                    conflict_size = conflict_size + 2;
                    if(conflict_size > conflict_size_max){
                        conflict_size_max = conflict_size;
                    }
                }
            }
        }
    }
    return f;
}

double TypeSearch(int src,int dst,string regex){ 
    RegexToDFA d;
    d.update(regex);
    Type = d.dfa;
    for(int i = 0; i < ConflictSet_type.size();i++){
        ConflictSet_type[i].resize(Type.stateNum);
    }
    for(int i = 0; i < Block.size();i++){
        Block[i].resize(Type.stateNum);
    }
    Bfs.update();
    BFS(src,dst);
    double time = Bfs.getTimerMilliSec();
    bool f = BFS_Arrive_b[src][Type.startState];
    if(f){
        Type_path(src,dst,Type.startState);
    }
    return time;
}

// void outputResult(string outfile,int i){
//     ofstream s;
//     s.open(outfile.c_str(),ios::app);
//     s <<  i << "th query:" << endl;
//     for(int i = 0; i < result.size(); i++){
//         s << result[i] << endl;
//     }
//     s.close();
// }

// void ResultTime(string outfile, double time,int i,double bfs_time){
//     ofstream s;
//     s.open(outfile.c_str(),ios::app);
//     s.setf(ios::fixed, ios::floatfield);  // 设定为 fixed 模式，以小数点表示浮点数
//     s.precision(4);  // 设置精度 4
//     s <<  i << " " << resultNumber << " " << time  <<  " " << bfs_time << endl;  //form : queryNumber,result number, time(s);
// }

void ReadData(string edgedata){
    ifstream infile(edgedata,ios::in);
    string line;

    // getline(infile,line);
    // istringstream ss(line);
    // ss >> numNode;
    // ss >> labelNum;
    // graph.resize(numNode + 1);
    // Back_graph.resize(numNode + 1);

    getline(infile,line);

    while(getline(infile, line)){
        istringstream ss(line);
        int src,dst,label;
        ss >> src;
        ss >> dst;
        ss >> label;
        //cout << src << " " << dst << " " << label << endl;

        if(numNode <= max(src,dst)){
            numNode = max(src,dst) + 1;
            graph.resize(numNode);
            Back_graph.resize(numNode);
        }
                
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
    Block.clear();
    ConflictSet_type.clear();
    resultNode_type.clear();
    resultLabel_type.clear();
    result.clear();
    BFS_Arrive_b.clear();
    passnode.clear();
}

void initialize_vector(){
    resultNumber = 0;
    bfs_count = 0;
    search_count = 0;
    search_count_max = 0;
    conflict_size = 0;
    conflict_size_max = 0;

    Block.resize(numNode);
    ConflictSet_type.resize(numNode);
    BFS_Arrive_b.resize(numNode);
}

int main(int argc, char* argv[]){

    // string edgeFile = argv[1];
    // string queryFile = edgeFile + "_Type_true.query";
    // string resultFile = edgeFile + "_Type_John_number2.result";

    string edgeFile = argv[1];

    // string edgeFile = "/home/liangqi/Projects/QiLiang/RSPQ/Datasets/BK/baidu";


    // string edgeFile = argv[1];
    // string queryFile = argv[2];
    // string resultFile = argv[3];
    // string TimeFile = argv[4];

    ReadData(edgeFile);//read data;
    cout<<"finishing reading data"<< endl;
    cout<<"node number: " << numNode << endl;
    cout<<"label number: " << labelNum << endl;

    // for(int type = 1; type < 7; type++){
    //     if(type == 1 || type == 4 || type == 6){
    //     // if(type == 6){
    //     string queryFile = edgeFile + "_Enum_Q" + to_string(type) + ".query";
    //     string resultFile = edgeFile + "_Enum_Q" + to_string(type) + "_ETRE.time.result";
    //     string memoryFile = edgeFile + "_Enum_Q" + to_string(type) + "_ETRE.memory.result";

    //     ifstream query;
    //     //query form : start end regex;
    //     query.open(queryFile.c_str());
    //     ofstream s;
    //     s.open(resultFile.c_str(),ios::out);

    //     ofstream s_m;
    //     s_m.open(memoryFile.c_str(),ios::out);

    //     string line;
    //     vector<string> temp;
    //     int i = 0;
    //     while(getline(query,line)){
    //         temp = Split(line,' ');
    //         long long src = stoll(temp[0]);
    //         long long dst = stoll(temp[1]);
    //         string regex = temp[2];
    //         i++;
    //         // cout << src << " " << dst << " " << regex << endl;
    //         initialize_vector();
    //         TC.update();
    //         double bfs_time = TypeSearch(src,dst,regex);
    //         double time = TC.getTimerMilliSec();
    //         //outputResult(resultFile,i);
    //         //ResultTime(TimeFile,time,i,bfs_time);
    //         // cout << i << " " << resultNumber << " " << time << endl;
    //         int memory = (search_count_max + bfs_count + conflict_size_max) * 4;
    //         s << i << " " << resultNumber << " " << time << endl;
    //         s_m << i << " " << bfs_count << " " << search_count_max << " " << conflict_size_max << " " << memory << endl;
    //         clear_vector();
    //     }
    //     query.close();
    //     s.close();
    //     s_m.close();
    //     }
    // }


        // string queryFile = edgeFile + "_Enum_Q" + to_string(type) + ".query";
        // string resultFile = edgeFile + "_Enum_Q" + to_string(type) + "_ETRE.time.result";
        // string memoryFile = edgeFile + "_Enum_Q" + to_string(type) + "_ETRE.memory.result";

        string queryFile = argv[2];
        string resultFile = argv[3];
        string memoryFile = argv[4];

        ifstream query;
        //query form : start end regex;
        query.open(queryFile.c_str());
        ofstream s;
        s.open(resultFile.c_str(),ios::out);

        ofstream s_m;
        s_m.open(memoryFile.c_str(),ios::out);

        string line;
        vector<string> temp;
        int i = 0;
        while(getline(query,line)){
            temp = Split(line,' ');
            long long src = stoll(temp[0]);
            long long dst = stoll(temp[1]);
            string regex = temp[2];
            i++;
            // cout << src << " " << dst << " " << regex << endl;
            initialize_vector();
            TC.update();
            double bfs_time = TypeSearch(src,dst,regex);
            double time = TC.getTimerMilliSec();
            //outputResult(resultFile,i);
            //ResultTime(TimeFile,time,i,bfs_time);
            // cout << i << " " << resultNumber << " " << time << endl;
            int memory = (search_count_max + bfs_count + conflict_size_max) * 4;
            s << i << " " << resultNumber << " " << time << endl;
            s_m << i << " " << bfs_count << " " << search_count_max << " " << conflict_size_max << " " << memory << endl;
            clear_vector();
        }
        query.close();
        s.close();
        s_m.close();

    return 0;
}