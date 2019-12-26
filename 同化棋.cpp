//需要在程序目录下创建qipan.txt！！！ 
#include <iostream>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <iomanip>
#include <ctime>
#include <algorithm>
#include <vector>
#include <fstream> 
//#include "jsoncpp/json.h" // C++编译时默认包含此库
#define USER 1
#define BOT 2
#define INF  666666666
#define AI_WIN  1000000
using namespace std;

const int dx[24] = { 0, 0, 1, -1, -1, 1, -1, 1, 2, 2, 2, 2, 2, -2, -2, -2, -2, -2, 1, 0, -1, 1, 0, -1};
const int dy[24] = { 1,-1, 0,  0,  1,-1, -1, 1, 2,-2, 1, 0, -1, 2, -2,  1,  0, -1, 2, 2,  2,-2,-2, -2};
int DEPTH; // 当前搜索层数
int stime;//开始时间
int dt[7][7][7][7];	//存储每种走法的类型

/*
棋盘格子
*/
struct Chess{
    int type; 
    bool isEmpty() {
        return type == 0 ? true : false;
    }
};

/*
棋盘
*/
struct ChessBoard{
    Chess chessBoard[7][7];
    ChessBoard() {
        for(int i = 0; i < 7; ++i)
            for(int j = 0; j < 7; ++j)
                chessBoard[i][j].type=0;
    }
    //复制棋盘
    ChessBoard(const ChessBoard &other) {
        for (int i = 0; i < 7; ++i)
            for (int j = 0; j < 7; ++j)
                chessBoard[i][j].type=other.chessBoard[i][j].type;
    }

    /*
    放置棋子是否成功
    */
    bool placePiece(int x, int y, int type) {
        if (chessBoard[x][y].isEmpty()) {
            chessBoard[x][y].type = type;
            int tx,ty;
            int nt = type == USER ? BOT : USER;
            for (int d = 0; d < 8;d++) {
                tx = x + dx[d];
                ty = y + dy[d];
                if(tx >= 0 && tx < 7 && ty >= 0 && ty < 7)
                    if (chessBoard[tx][ty].type == nt)
                        chessBoard[tx][ty].type = type;
            }
            return true;
        }
        return false;
    }

    /*
    拿掉棋子是否成功
    */
     bool removePiece(int x, int y,int type) {
        if (chessBoard[x][y].isEmpty() || chessBoard[x][y].type == type) {
            chessBoard[x][y].type = 0;
            return true;
        }
        return false;
    }
};

ChessBoard curState; // 当前棋盘

int nextType(int type) {
    return type==1? 2:1;
}
/*
当前行棋人能否放置棋子
*/
bool placePiece(const int &curUser,const int &f,const int &fx,const int &fy) {
    int x = fx + dx[f];
    int y = fy + dy[f];
    if (curState.placePiece(x, y, curUser)) {
        if (f > 7) {
            if (!curState.removePiece(fx,fy,curUser))
                return false;
        }
        return true;
    }
    return false;
}
/*
判定游戏是否结束
*/
int isOver(int type) {
    int cnt[3] = {0};
    for (int i = 0;i < 7;i++)
        for (int j = 0;j < 7;j++) {
            cnt[curState.chessBoard[i][j].type]++;
            if ((!type) && cnt[0] > 0 && cnt[1] > 0 && cnt[2] > 0)
                return 0;
        }
    if (cnt[1] == 0)
        return 2;
    if (cnt[2] == 0)
        return 1;
    if ((type || (cnt[0] == 0)) && cnt[1] >= cnt[2])
        return 1;
    if ((type || (cnt[0] == 0)) && cnt[1] < cnt[2])
        return 2;
    return 0;
}

/*
判断X,Y位置是否可以搜
*/
bool canSearch(const ChessBoard &state,const int &x,const int &y) {
    if (state.chessBoard[x][y].type == 0 &&
        x >= 0 && x < 7 &&
        y >= 0 && y < 7
        )
        return true;
    return false;
}

//计算当前局面的估价
int evaluate(const ChessBoard &state,const int &type) {
    int nt = nextType(type);
    int ans1 = 0,ans2 = 0;
    for (int i = 0; i < 7; ++i)
        for (int j = 0; j < 7; ++j) {
            if (state.chessBoard[i][j].type == type)
                ans1++;
            else
                if (state.chessBoard[i][j].type == nt)
                    ans2++;
        }
    if (ans1 > 24) //自己超过半数棋子
        return AI_WIN - 100 + ans1 - ans2;
    if (ans2 > 24) //对手超过半数棋子
        return -AI_WIN + 100 + ans1 - ans2 ;
    return ans1 - ans2;
}

/*
极大极小搜索
*/
int MinMax(const ChessBoard &state,const int &curUser,const int &f,const int &fx,const int &fy,const int &type,const int &depth,const int &alpha,const int &beta) {
    ChessBoard newState(state);//复制 
    int x = fx + dx[f];
    int y = fy + dy[f];
    newState.placePiece(x, y, nextType(type));
    if (f > 7)
        newState.removePiece(fx, fy, nextType(type));
    int weight = 0;
    int max = -INF; 
    int min = INF; 
    if(depth<=0) {
        weight = evaluate(newState, curUser); // 评估当前方局面
        return weight; /
    }

    int count = 0; 
    for (int i = 0; i < 7; ++i) {
        for (int j = 0; j < 7; ++j){
            if (newState.chessBoard[i][j].type == type ) //找出所有备选点
                for (int d = 0; d < 24;d++){ 
                    int tx = i + dx[d];
                    int ty = j + dy[d];
                    if (canSearch(newState, tx, ty)) { //可以走则搜索
                        count++;
                        weight = MinMax(newState, curUser, d, i, j, nextType(type),depth-1,min,max);//根据选择拓展出下一层
                        if (weight > max)
                            max = weight; // 更新下层上界
                        if (weight < min)
                            min = weight; // 更新下层下界
//                            //剪枝
//                            if (type == curUser) { 
//                                if (max >= alpha)
//                                    return max;
//                            }
//                            else {                  
//                                if (min <= beta)
//                                    return min;
//                            }
                    }
                }
        }
    }
    if (count == 0){//无路可走
        if (int ov = isOver(1)) {
            if (ov == curUser)
                return AI_WIN; // 我方胜
            if (ov != curUser)
                return -AI_WIN; // 对方胜
        }
    }
    if (type == curUser)
        return max; // 最大层给出最大值
    else
        return min; // 最小层给出最小值
    
}

//保存选择
struct choice{
    int f,fx,fy;//方向 始发点坐标
    choice(){}
    void s(int a,int b,int c) {
        f = a;
        fx = b;
        fy = c;
    }
};
/*
AI 行棋
*/
choice placePieceAI(const int &curUser) {
    int weight;
    int max = -INF; 
    int min = INF; 
    int fd = -1,fx,fy; //最优选择
    stime=clock(); //开始时间
    for (DEPTH = 1;DEPTH < 80;DEPTH++) { //迭代加深 
        if(clock() - stime> CLOCKS_PER_SEC * 0.8) //到达限定时间则退出
            break;
        for (int i = 0; i < 7; ++i) {
            for (int j = 0; j < 7; ++j)
                if (curState.chessBoard[i][j].type == curUser) //找出所有备选点
                    for (int d = 0; d < 24;d++){ //枚举走法
                        if(clock() - stime> CLOCKS_PER_SEC * 0.8) 
                            break;
                        int tx = i + dx[d];
                        int ty = j + dy[d];
                        if (canSearch(curState, tx, ty)) { //可以走则搜索
                            weight = MinMax(curState, curUser,d, i, j, nextType(curUser), DEPTH, min, max);
                            if (weight > max) {
                                max = weight; // 更新下层上界
                                fd = d; //更新最优记录
                                fx = i;
                                fy = j;
                            }
                            if (weight < min)
                                min = weight;
                        }
                }
        }
    }
    choice ans;
    //记录并执行最优点
    ans.s(fd,fx,fy);
    if (fd != -1)
        placePiece(curUser,fd,fx,fy);
    return ans; 
}

void show() {
 	system("cls");
 	cout << "        ０１２３４５６" << endl;
	cout << "      ╔╤╤╤╤╤╤╤╗" << endl;
	for (int l=0; l<7; l++){
		 cout << "     " <<l<< "╟";
		for (int k=0; k<7; k++)
		{
			switch (curState.chessBoard[l][k].type)
			{   case 0: cout << "┼"; break;
			    case 1: cout << "◎"; break;
				case 2: cout << "●"; break;
			}
		}
		cout << "╢" << endl;
	}
	cout << "      ╚╧╧╧╧╧╧╧╝" << endl;
}

int main(){
	istream::sync_with_stdio(false);
	system("title 同化棋by朱政烨");
    system("color 0B"); //设置颜色 
    
    int tx,ty,cu,ov;
    choice cs;
    int x1,y1,x,y;
    for (int x0 = 0;x0 < 7;x0++)
        for (int y0 = 0;y0 < 7;y0++) {
            for (int d = 0;d < 24;d++) {
                x1 = x0 + dx[d];
                y1 = y0 + dy[d];
                if (x1 >= 0 && x1 < 7 && y1 >=0 && y1 < 7)
                    dt[x0][y0][x1][y1] = d;
            }
        }
        
    cout<<"\n\t                             ************\n";
	cout<<"                                        同化棋\n";
	cout<<"                                     ************\n   \n";
	cout<<"                 ┏━━━━━━━━━━━━━━━━━━━━━━━┓\n";  
	cout<<"                 ┃                     欢迎！                   ┃\n";
	cout<<"                 ┣━━━━━━━━━━━━━━━━━━━━━━━┫\n";
	cout<<"                 ┃                 ① 从头开始                  ┃\n"; 
	cout<<"                 ┣━━━━━━━━━━━━━━━━━━━━━━━┫\n";
	cout<<"                 ┃                 ② 读取存档                  ┃\n";
	cout<<"                 ┣━━━━━━━━━━━━━━━━━━━━━━━┫\n";
	cout<<"                 ┃                 ③ 退出游戏                  ┃\n";
	cout<<"                 ┗━━━━━━━━━━━━━━━━━━━━━━━┛\n"<<endl;
    cout<<"请输入数字进行选择："<<endl; 
    int number;
	cin>>number;
	while(number<1||number>3){
		cout<<"请重新输入一个1-3的数字："<<endl;
		cin>>number;
	} 
	if(number==3) return 0;
	if(number==1){
	    curState.chessBoard[0][0].type = USER;
	    curState.chessBoard[6][6].type = USER;
	    curState.chessBoard[6][0].type = BOT;
	    curState.chessBoard[0][6].type = BOT;
	    cu=1;
	}
	else{
		ifstream fin("qipan.txt");
		fin>>cu;
		for(int i=0;i<7;i++)
			for(int j=0;j<7;j++)
				fin>>curState.chessBoard[i][j].type;
	}
	while(1){       
            show();
            cout<<"输入起点和终点坐标：(必须合法！)"<<endl; 
            cin>>tx>>ty>>x>>y;
            placePiece(cu,dt[tx][ty][x][y],tx,ty);
			show();
			cout<<"自动保存中"<<endl; 
            cu=nextType(cu);
            cs=placePieceAI(cu);
            if (ov=isOver(0)){
                show();
                printf("%s WIN!\n",ov == 1?"黑方":"白方");
                break;
            }
            cu=nextType(cu);
            ofstream fout("qipan.txt",ios::trunc);//清空棋盘内容
			fout<<cu<<endl;
			for(int i=0;i<7;i++)
			    for(int j=0;j<7;j++)
				  fout<<curState.chessBoard[i][j].type<<endl; 
			fout.close(); 
	} 
    system("pause");
    return 0;
}

