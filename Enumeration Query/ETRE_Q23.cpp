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

//delete and recover graph;
vector<edge> recover_graph;

vector<string> Split(const string& s, const char delim = '\t') {
	vector<string> elements;
	stringstream ss(s);
	string item;
	while (getline(ss, item, delim)) {
		elements.push_back(item);
	}
	return elements;
}

vector<string> RegexDivide(string str){
    int size = str.length();
    string pre;
    string type;
    string suff;
    vector<string> regex;
    int i = size - 1;
    bool s = true;
    bool p = false;
    bool t = false;
    while(i != 0){
        if(str[i] == '*'){
            t = true;
            s = false;
        }
        if(s){
            if(str[i] == '&' && str[i-1] == '*'){
                //
            }
            else{
                suff = str[i] + suff;
            }
        }
        if(t){
            if(str[i] == '&' && str[i-1] != '*'){
                p = true;
                t = false;
                i = i - 1;
            }else{
                type = str[i] + type;
            }
        }
        if(p){
            pre = str[i] + pre;
        }
        i--;
    }
    if(s){
        suff = str[i] + suff;
    }
    if(p){
        pre = str[i] + pre;
    }
    if(t){
        type = str[i] + type;
    }
    string str1 = "";
    string temp = "";
    for(int i = 0;i < suff.length();i++){
        if(suff[i] == '&'){
            str1 = suff[i] + temp + str1;
            temp = "";
        }else{
            temp = temp + suff[i];
        }
    }
    suff = temp + str1;
    regex.emplace_back(pre);
    regex.emplace_back(type);
    regex.emplace_back(suff);
    return regex;
}

void DeleteGraph(int src){
    // for(auto it = graph[src].begin(); it != graph[src].end();){
    //     int dst = it->toNode;
    //     int label = it->label;
    //     edge temp;
    //     temp.label = label;
    //     temp.toNode = dst;
    //     recover_graph.push_back(temp);
    //     graph[src].erase(it);
    // }
    recover_graph = graph[src];
    graph[src].clear();
}

void recover(int src){
    graph[src] = recover_graph;
    recover_graph.clear();
}

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
    Block.clear();
    ConflictSet_type.clear();
    resultNode_type.clear();
    resultLabel_type.clear();
    result.clear();
    BFS_Arrive_b.clear();
    passnode.clear();
}

void initialize_vector(){
    // resultNumber = 0;
    resultNumber = 0;
    Block.resize(numNode);
    ConflictSet_type.resize(numNode);
    BFS_Arrive_b.resize(numNode);
}

void ETRE(int node, int dst, string regex){

    vector<string> regex_result = RegexDivide(regex);
    vector<int> node_set;
    
    string regex_pre = "";
    for(int i = 1; i < regex_result[0].size() - 1; i++){
        regex_pre += regex_result[0][i];
    }
    int pre = stol(regex_pre);

    for(auto it = graph[node].begin(); it != graph[node].end(); it++){
        if(it->label == pre){
            node_set.push_back(it->toNode);
        }
    }

    DeleteGraph(node);

    TC.update();
    if(!node_set.empty()){
        string type_regex = regex_result[1];
        for(int i = 0; i < node_set.size(); i++){
            if(TC.getTimerSecond() > 600 || resultNumber > 1000){
                recover(node);
                return;
            }
            int src_s = node_set[i];
            if(src_s == dst){
                resultNumber++;
                continue;
            }
            initialize_vector();
            TypeSearch(src_s,dst,type_regex);         
            clear_vector();
        }
    }
    recover(node);
}

int main(int argc, char* argv[]){

    // string edgeFile = argv[1];
    // string queryFile = edgeFile + "_Type_true.query";
    // string resultFile = edgeFile + "_Type_John_number2.result";

    // string edgeFile = "/home/liangqi/Projects/QiLiang/RSPQ/Datasets/YT/Youtube";
    // string queryFile = "/home/liangqi/Projects/QiLiang/RSPQ/Text_data/query.txt";
    // string resultFile = "/home/liangqi/Projects/QiLiang/RSPQ/Text_data/result.txt";

    string edgeFile = argv[1];
    ReadData(edgeFile);//read data;
    cout<<"finishing reading data"<< endl;
    cout<<"node number: " << numNode << endl;
    cout<<"label number: " << labelNum << endl;

        // for(int type = 2; type < 4; type++){

        // string queryFile = edgeFile + "_Enum_Q" + to_string(type) + ".query";
        // string resultFile = edgeFile + "_Enum_Q" + to_string(type) + "_ETRE.time.result";
        // string memoryFile = edgeFile + "_Enum_Q" + to_string(type) + "_ETRE.memory.result";

        // ifstream query;
        // //query form : start end regex;
        // query.open(queryFile.c_str());

        // ofstream s;
        // s.open(resultFile.c_str(),ios::out);

        // ofstream s_m;
        // s_m.open(memoryFile.c_str(),ios::out);

        // string line;
        // vector<string> temp;
        // int i = 0;
        // while(getline(query,line)){
        //     temp = Split(line,' ');
        //     long long src = stoll(temp[0]);
        //     long long dst = stoll(temp[1]);
        //     string regex = temp[2];
        //     i++;
        //     // cout << src << " " << dst << " " << regex << endl;
        //     initialize_vector();
        //     ETRE(src,dst,regex);
        //     double time = TC.getTimerMilliSec();
        //     //outputResult(resultFile,i);
        //     //ResultTime(TimeFile,time,i,bfs_time);
        //     // cout << i << " " << resultNumber << " " << time << endl;
        //     s << i << " " << resultNumber << " " << time << endl;
        //     clear_vector();
        // }
        // query.close();
        // s.close();
        // s_m.close();

        // }



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
        ETRE(src,dst,regex);
        double time = TC.getTimerMilliSec();
        //outputResult(resultFile,i);
        //ResultTime(TimeFile,time,i,bfs_time);
        // cout << i << " " << resultNumber << " " << time << endl;
        s << i << " " << resultNumber << " " << time << endl;
        clear_vector();
    }
    query.close();
    s.close();
    s_m.close();


    return 0;
}