#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <string>
#include <algorithm>
#include <sstream>
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <string.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>
#include "RegexToDFA.h"
#include "Struct_type.h"
#include "TimerClock.h"

using namespace std;

FinalDFA dfa;
TimerClock TC;
int numNode = 0;
int numEdge = 0;
int labelNum = 0;
int search_count = 0;
vector<vector<edge>> graph;
vector<vector<edge>> Back_graph;
// vector<unordered_set<int>> labelIn;
// vector<unordered_set<int>> labelOut;
// unordered_map<long long, int> v_index1;
// unordered_map<int, long long> v_index2;//node mapping;
vector<double> memory_use;

vector<unordered_map<int,bool> > BFS_Arrive_f;
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

int parseLine(char* line){
    // This assumes that a digit will be found and the line ends in " Kb".
    int i = strlen(line);
    const char* p = line;
    while (*p <'0' || *p > '9') p++;
    line[i-3] = '\0';
    i = atoi(p);
    return i;
}

double getValue(){ // This value is in KB!
    #ifdef __MACH__

    struct task_basic_info t_info;
    mach_msg_type_number_t t_info_count = TASK_BASIC_INFO_COUNT;

    if (KERN_SUCCESS != task_info(mach_task_self(),
                                TASK_BASIC_INFO, (task_info_t)&t_info, 
                                &t_info_count))
    {
        return -1;
    }
    return (double)t_info.virtual_size/1024;
    #else
    FILE* file = fopen("/proc/self/status", "r");
    int result = -1;
    char line[128];

    while (fgets(line, 128, file) != NULL){
        if (strncmp(line, "VmSize:", 7) == 0){
            result = parseLine(line);
            break;
        }
    }
    fclose(file);
    return (double)result;
    #endif
}

void ReadData(string edgedata){
    ifstream infile(edgedata,ios::in);
    string line;
    getline(infile,line);
    istringstream ss(line);
    ss >> numNode;
    ss >> labelNum;

    // numNode = 297000000;

    graph.resize(numNode + 1);
    Back_graph.resize(numNode + 1);
    while(getline(infile, line)){
        istringstream ss(line);
        int src,dst,label;
        ss >> src;
        ss >> dst;
        ss >> label;
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
};

int bbfs(int src,int dst){
    Q_F.push(make_pair(src,dfa.startState));
    BFS_Arrive_f[src][dfa.startState] = true;
    for(auto it = dfa.endStates.begin();it != dfa.endStates.end();it++){
        Q_B.push(make_pair(dst,*it));
        BFS_Arrive_b[dst][*it] = true;
    }
    while(!Q_F.empty() && !Q_B.empty()){
        int node = Q_B.front().first;
        int state = Q_B.front().second;
        Q_B.pop();
        for(auto it = Back_graph[node].begin();it != Back_graph[node].end();it++){
            int label = it->label;
            if(dfa.Back_labelToState[state].count(label)){
                int nextnode = it->toNode;
                for(auto iter = dfa.Back_labelToState[state][label].begin();iter != dfa.Back_labelToState[state][label].end();iter++){
                    int nextstate = *iter;
                    if(!BFS_Arrive_b[nextnode].count(nextstate)){
                        search_count += 2;
                        Q_B.push(make_pair(nextnode,nextstate));
                        BFS_Arrive_b[nextnode][nextstate] = true;
                    }
                    if(BFS_Arrive_f[nextnode].count(nextstate)){
                        return 1;
                    }
                }
            }
        }

        node = Q_F.front().first;
        state = Q_F.front().second;
        Q_F.pop();
        for(auto it = graph[node].begin();it != graph[node].end();it++){
            int label = it->label;
            if(dfa.labelToState[state].count(label)){
                int nextnode = it->toNode;
                int nextstate = dfa.labelToState[state][label];
                if(!BFS_Arrive_f[nextnode].count(nextstate)){
                    search_count += 2;
                    Q_F.push(make_pair(nextnode,nextstate));
                    BFS_Arrive_f[nextnode][nextstate] = true;
                }
                if(BFS_Arrive_b[nextnode].count(nextstate)){
                    return 1;
                }
            }
        }
    }
    return 0;
}

int ReachSearch(int src,int dst,string regex){
    RegexToDFA d;
    d.update(regex);
    dfa = d.dfa;
    int result;
    result = bbfs(src,dst);
    return result;
}

// void ResultTime(string outfile,double time,int i,int result){
//     ofstream s;
//     s.open(outfile.c_str(),ios::app);
//     s.setf(ios::fixed, ios::floatfield);  // 设定为 fixed 模式，以小数点表示浮点数
//     s.precision(4);  // 设置精度 4
//     s <<  i << " " << result << " " << time  << endl;
// }

void init(){
    while(!Q_B.empty()){
        Q_B.pop();
    }
    while(!Q_F.empty()){
        Q_F.pop();
    }
    BFS_Arrive_b.clear();
    BFS_Arrive_f.clear();
    BFS_Arrive_b.resize(numNode);
    BFS_Arrive_f.resize(numNode);
}

void RunBBFS(string queryfile,string resultfile, string memoryfile){
    ifstream query;
    query.open(queryfile.c_str());
    string line;
    vector<string> temp;
    int i = 0;
    ofstream s;
    s.open(resultfile.c_str(),ios::out);
    s.setf(ios::fixed, ios::floatfield);  // 设定为 fixed 模式，以小数点表示浮点数
    s.precision(4);  // 设置精度 4

    ofstream s_m;
    s_m.open(memoryfile.c_str(),ios::out);
    s_m.setf(ios::fixed, ios::floatfield);  // 设定为 fixed 模式，以小数点表示浮点数
    s_m.precision(4);  // 设置精度 4

    while(getline(query,line)){
        temp = Split(line,' ');
        long long src = stoll(temp[0]);
        long long dst = stoll(temp[1]);
        string regex = temp[2];
        //cout << src << " " << dst << " " << regex << endl; 
        i++;
        search_count = 0;
        init();
        TC.update();
        int result = ReachSearch(src,dst,regex);
        double time = TC.getTimerMicroSec();
        double memory = (search_count * 4);

        s <<  i << " " << result << " " << time  << endl;
        s_m << i << " " << memory << endl;
    }
    query.close();
    s.close();
    s_m.close();
}

// void ResultMemory(string outfile){
//     ofstream s;
//     s.open(outfile.c_str(),ios::out);
//     s.setf(ios::fixed, ios::floatfield);  // 设定为 fixed 模式，以小数点表示浮点数
//     s.precision(4);  // 设置精度 4
//     for(int i = 0; i < memory_use.size();i++){
//         s << i+1 << " " << memory_use[i] << endl;
//     }
//     s.close();
// }

// int main(){

//     string edgeFile = "/home/QiLiang/projects/Data_Text/graph.txt";
//     string queryFile = "/home/QiLiang/projects/Data_Text/query_bbfs.txt";
//     string resultFile = "/home/QiLiang/projects/Data_Text/result.txt";
//     string memoryfile = "/home/QiLiang/projects/Data_Text/memory.txt";

//     ReadData(edgeFile);//read data;
//     // cout<<"finishing reading data"<< endl;
//     // cout<<"node number: " << numNode << endl;
//     // cout<<"edge number: " << numEdge << endl;
//     // cout<<"label number: " << labelNum << endl;

//     RunBBFS(queryFile,resultFile);
//     ResultMemory(memoryfile);
//     return 0;
// }

int main(int argc, char* argv[]){

    // string edgeFile = argv[1];
    // string queryFile = edgeFile + "_LCR_bbfs.query";
    // string resultFile = edgeFile + "_LCR_us.result";

    string edgeFile = argv[1];
    // string edgeFile = "/home/liangqi/Projects/QiLiang/RSPQ/Datasets/AD/advogato";

    ReadData(edgeFile);//read data;
    cout<<"finishing reading data"<< endl;
    cout<<"node number: " << numNode << endl;
    cout<<"edge number: " << numEdge << endl;
    cout<<"label number: " << labelNum << endl;


    // for(int type = 1; type < 7; type++){
    //     if(type == 1 || type == 4){
    //         string queryFile = edgeFile + "_Reach_Q" + to_string(type) + "_bbfs.query";
    //         string resultFile = edgeFile + "_Reach_Q" + to_string(type) + "_BBFS.time.result";
    //         string memoryFile = edgeFile + "_Reach_Q" + to_string(type) + "_BBFS.memory.result";
    //         // string queryFile = edgeFile + "_bbfs_Q" + to_string(type) + "_true.query";
    //         // string resultFile = edgeFile + "_Reach_Q" + to_string(type) + "_BBFS_true.result";
    //         RunBBFS(queryFile,resultFile,memoryFile);
    //     }
    // }

    // string queryFile = "/home/liangqi/Projects/QiLiang/RSPQ/Datasets/SO/stackoverflow_Type_bbfs.query";
    // string resultFile = "/home/liangqi/Projects/QiLiang/RSPQ/Datasets/SO/stackoverflow_Reach_Q6_BBFS.time.result";
    // string memoryFile = "/home/liangqi/Projects/QiLiang/RSPQ/Datasets/SO/stackoverflow_Reach_Q6_BBFS.memory.result";
    // RunBBFS(queryFile,resultFile,memoryFile);

    
    string queryFile = argv[2];
    string resultFile = argv[3];
    string memoryfile = argv[4];

    RunBBFS(queryFile,resultFile,memoryFile);

    return 0;
}
