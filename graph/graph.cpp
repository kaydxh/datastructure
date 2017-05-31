#include <stdio.h>
#include <iostream>
#include <vector>
#include <queue>
#include <stack>
using namespace std;

typedef enum {UNDISCOVERED, DISCOVERED, VISITED} VStatus;
typedef enum {UNDETEMINED, TREE, CROSS, FORWARD, BACKWARD} EType;

#define INT_MAX int((unsigned)~0 >> 1)
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

template <typename Tv, typename Te>
class Graph {
private:
	void reset() { //所有顶点、边的辅助信息复位
		for (int i = 0; i < n; ++i) { //所有顶点复位
			status(i) = UNDISCOVERED;
			dTime(i) = fTime(i) = -1;
			parent(i) = -1;
			priority(i) = INT_MAX; //优先级最低

			for (int j = 0; j < n; ++j) { //所有边复位
				if (exists(i, j)) {
					type(i, j) = UNDETEMINED;
				}
			}
		}
	}

private:
	void BFS (int, int&);
	void DFS (int, int&);
	bool TSort ( int, int&, stack<Tv>* ); //（连通域）基于DFS的拓扑排序算法
	void BCC ( int, int&, stack<int>& ); //（连通域）基于DFS的双连通分量分解算法

	template <typename PU>
	void PFS(int, PU);
public:
	//顶点
	int n; //顶点总数
	virtual Tv& vertex(int) = 0;
	virtual int inDegree(int) = 0;
	virtual int outDegree(int) = 0;
	virtual int firstNbr(int) = 0;
	virtual int nextNbr(int, int) = 0;
	virtual VStatus& status(int) = 0;
	virtual int& dTime(int) = 0;
	virtual int& fTime(int) = 0;
	virtual int& parent(int) = 0;
	virtual int& priority(int) = 0;
	virtual void insert(Tv const& ) = 0;
	virtual Tv remove(int) = 0;

	//边
	int e; //边总数
	virtual bool exists(int, int) = 0;
	virtual void insert(Te const&, int, int , int) = 0;
	virtual Te remove(int, int) = 0;
	virtual EType& type(int, int) = 0;
	virtual Te& edge(int, int) = 0;
	virtual int& weight(int, int) = 0;

	//算法
	void bfs(int); // 广度优先搜索算法
	void dfs(int); // 深度优先搜索算法
	stack<Tv>* tSort ( int ); //基于DFS的拓扑排序算法
	void bcc ( int ); //基于DFS的双连通分量分解算法

	template <typename PU>
	void pfs ( int, PU ); //优先级搜索框架
}; 

//广度优先搜索：越早被访问的顶点，其邻居越优先被选用。以顶点s为基点，再依次访问s所有尚未访问的邻居，
//再按后者被访问的先后次序，逐个访问它们的邻居。
// 时间复杂度为O(n+e)，首先花费O(n+e)时间复位所有顶点和边的状态。bfs()本身对所有顶点的枚举为O(n)，
//而在对BFS()的所有调用中，每个顶点，每条边均只消耗O(1)，累计O(n+e)。综合O(n+e)的时间。
template <typename Tv, typename Te>
void Graph<Tv, Te>::bfs(int s) { // 0 <= s <n
	reset(); //复位顶点和边的状态
	int clock = 0;
	int v = s; 

	do {
		if (UNDISCOVERED == status(v)) { //一旦遇到未发现的顶点，就从该顶点出发启动一次BFS
			BFS(v, clock);
		}

	} while(s != (v = (++v % n))); //按序号遍历
}

template <typename Tv, typename Te>
void Graph<Tv, Te>::BFS(int v, int& clock) {
	queue<int> q;

	status(v) = DISCOVERED; // 起点v是肯定已经发现了
	q.push(v);

	while(!q.empty()) {
		int cv = q.front(); //取出队首元素
		q.pop();

		dTime(cv) = ++clock; 

		for (int u = firstNbr(cv); -1 < u; u = nextNbr(cv, u)) { //枚举cv的所有邻居
			if (UNDISCOVERED == status(u)) { //如果cv的邻居是还没有被发现，就设置为发现，并加入队列中。
				status(u) = DISCOVERED; 
				q.push(u);
				type(cv, u) = TREE; //边(cv, u) 类型为TREE
				parent(u) = cv; //顶点u的父亲是cv
			} else {
				type(cv, u) = CROSS; // 如果顶点u已经被发现或者访问过了，那这(cv, u)就是跨边
			}
		}

		status(cv) = VISITED; //遍历完cv的所有邻居后，v就是访问过的顶点了
		printf("%c ", vertex(cv));
	}
} 

//深度优先搜索：优先选取最后一个被访问的顶点的邻居, 以顶点s为基点的DFS搜索，将首先访问s,
//再从s所有还没有访问的邻居中任取其一，并以之为基点，递归执行DFS搜索
//时间复杂度为O(n+e)。首先需要花费O(n+e)时间对所有顶点和边的状态复位。dfs()本身对所有顶点的枚举
//需要O(n)时间。每个顶点、每条边只在子函数DFS()的某一递归实例中耗费O(1)时间。这里不计算递归函数消耗。综合
//时间复杂度为O(n+e)。
template <typename Tv, typename Te>
void Graph<Tv, Te>::dfs(int s) {
	reset(); //复位顶点和边的状态
	int clock = 0;
	int v = s;

	do {
		if (UNDISCOVERED == status(v)) { //一旦遇到未发现的顶点，就从该顶点出发启动一次DFS
			DFS(v, clock);
		}

	} while(s != (v = (++v % n))); //按序号遍历
}

template <typename Tv, typename Te>
void Graph<Tv, Te>::DFS(int v, int& clock) {
	dTime(v) = ++clock; 
	status(v) = DISCOVERED; //发现当前顶点

	for (int u = firstNbr(v); -1 < u; u = nextNbr(v, u)) {//枚举v的所有邻居
		switch (status(u)) {
			case UNDISCOVERED: { //u如果是未发现的，就以此为新基点，递归DFS
				type(v, u) = TREE;
				parent(u) = v;
				DFS(u, clock);
			} break;

			case DISCOVERED: { // u如果是已经发现的，就改变边(v,u)的状态
				type(v, u) = BACKWARD; //向后边，指子代指向祖先的边
			} break;

			default:// u是已经访问过的，则依据v和u哪个先被发现，如果v先发现（dTime(v) < dTime(u)，那么边(v,u)就是向前边。
				type(v, u) = (dTime(v) < dTime(u)) ? FORWARD : CROSS;//否则就是跨边。向前边就是祖父指向子代的边，跨边就是2个顶点不存在直接相连的边
				break;
		}

	}

	status(v) = VISITED; //v访问结束
	fTime(v) = ++clock; // 一次DFS，fTIme增加一次
	printf("%c ", vertex(v));	
}

//基于DFS的拓扑排序：其顺序是DFS的逆序，如果DFS中存在向后边，就退出，因为如果存在向后边，就表示有环，不可能存在拓扑排序。
//时间复杂度为O(n)
template <typename Tv, typename Te>
stack<Tv> * Graph<Tv, Te>::tSort(int s) {
	reset();
	int clock = 0;
	int v = s;

	stack<Tv> *pvs = new(std::nothrow) stack<Tv>; //用栈记录排序顶点，在调用端释放

	do {
		if (UNDISCOVERED == status(v)) { //如果v是还未发现的顶点，那么就TSort，如果TSort返回false（即存在后向边，这种情况是不存在拓扑排序的），
			if (!TSort(v, clock, pvs)) {//需要要该stack清空,并结束计算，返回。
				while (!pvs->empty()) {
					pvs->pop();
				}
				break; // 退出循环，返回。
			}
		}

	} while(s != (v = (++v %n)));

	return pvs;
}

template <typename Tv, typename Te>
bool Graph<Tv, Te>::TSort(int v, int& clock, stack<Tv>* pvs) {
	dTime(v) = ++clock;
	status(v) = DISCOVERED;

	for (int u = firstNbr(v); -1 < u; u = nextNbr(v, u)) {
		switch (status(u)) {
			case UNDISCOVERED: {
				parent(u) = v;
				type(v, u) = TREE;

				if (!TSort(u, clock, pvs)) {
					return false;
				}
			} break;

			case DISCOVERED: {
				type(v, u) = BACKWARD; //一旦发现后向边（非DAG，不是有向无环图），则返回。因为存在环路，所以不存在拓扑排序
				return false;
			}

			default: 
				type(v, u) = (dTime(v) < dTime(u)) ? FORWARD: CROSS;
				break;
		}
	}

	status(v) = VISITED;
	fTime(v) = ++clock;
	pvs->push(vertex(v));

	return true;
}

//若节点C的移除导致其某一棵真子树与其真祖先之间无法连通，则C必为关节点。
//反之，若C的所有真子树都能与C的某一真祖先连通，则C就不可能是关节点。
template <typename Tv, typename Te>
void Graph<Tv, Te>::bcc(int s) {
	reset();
	int clock = 0;
	int v = s;

	stack<int> vs;

	do {
		if (UNDISCOVERED == status(v)) {
			BCC(v, clock, vs);
			vs.pop();
		}

	} while(s != (v = (++v % n)));
}

#define hca(x) (fTime(x)) //利用此处闲置的fTime[]充当hca[]，hca(v) 即v所能连通的最高祖先
template <typename Tv, typename Te>
void Graph<Tv, Te>::BCC(int v, int& clock, stack<int>& vs) {
	hca(v) = dTime(v) = ++clock;
	status(v) = DISCOVERED;
	vs.push(v);

	for (int u = firstNbr(v); -1 < u; u = nextNbr(v, u)) {

		switch(status(u)) {
			case UNDISCOVERED: {
				parent(u) = v;
				type(v, u) = TREE;
				BCC(u, clock, vs); //从顶点u处深入

				if (hca(u) < dTime(v)) { //遍历返回后，如果u的所能连通的最高祖先比v要高（即通过后向边指向v的真祖先）
					hca(v) = MIN(hca(v), hca(u));// 更新v所能连通的最高祖先
				} else { // 否则v就是关节点(u以下就是一个BCC，且其中顶点此时正集中于栈的顶部)
					while(v != vs.top()) { //弹出当前BCC中的节点, 除了关节点v
						printf("%c - ", vertex(vs.top())); //打印连通图中的边
						vs.pop();
					}

					printf("%c\n", vertex(v)); //打印最后一个关节点
				}

			} break;

			case DISCOVERED: {
				type(v, u) = BACKWARD; //向后边，u是v的祖先
				if (u != parent(v)) { // u不是v的父亲，而应该是真祖先
					hca(v) = min(hca(v), dTime(u)); //更新hca(v),按照越小越高的准则
				}
			} break;

			default: //only 有向图
				type(v, u) = (dTime(v) < dTime(u)) ? FORWARD : CROSS;
				break;

		}
	
	}

	status(v) = VISITED;
}

//优先级搜索：不妨约定优先级数越大的顶点，优先级越低。借助函数对象prioUpdater，按照不同的
//优先级策略实现不同的算法。
//时间复杂度O(n²), PFS有2重循环构成。若采用领接表实现方式，同时假定prioUpdater()只需要常数时间
// 外层循环的时间复杂度为O(n)，内层循环的时间复杂度为O(n),综合，总的时间复杂度为O(n²)
template <typename Tv, typename Te>
template <typename PU>
void Graph<Tv, Te>::pfs(int s, PU prioUpdater) {
	reset();
	int v = s;

	do {
		if (UNDISCOVERED == status(v)) {//一旦遇到尚未发现的顶点，就从该顶点出发启动一次PSF
			PFS(v, prioUpdater);
		}
	} while( s != (v = (++v % n))); //按序号遍历
}

template <typename Tv, typename Te>
template <typename PU>
void Graph<Tv, Te>::PFS(int s, PU prioUpdater) {
	priority(s) = 0;
	status(s) = VISITED;
	parent(s) = -1;

	while (1) {
		for (int w = firstNbr(s); -1 < w; w = nextNbr(s, w)) { //枚举s的所有邻居w，更新顶点w的优先级
			prioUpdater(this, s, w); // 时间复杂度O(e)
		}

		for (int shortest = INT_MAX, w = 0; w < n; w++) {//从所有顶点中找出尚未被发现的顶点，
			if (UNDISCOVERED == status(w)) { // 并在这些尚未被发现的顶点中找出优先级最高的顶点s
				if (shortest > priority(w)) {// 时间复杂度O(n)
					shortest = priority(w);
					s = w;
				}
			}
		}

		if (VISITED == status(s)) {//如果s是被访问过的，就说明所有顶点都加入到遍历树中，所以退出
			break;
		}

		status(s) = VISITED; //否则，将s的状态设置为VISITED，并将s的父亲到s的边设置为TREE
		type(parent(s), s) = TREE;
	}
}


template <typename Tv>
struct Vertex {
	Tv data;
	int inDegree;
	int outDegree;
	VStatus status;
	int dTime;
	int fTime;
	int parent;
	int priority;

	Vertex(Tv const& d = Tv(0)):
		data(d),
		inDegree(0),
		outDegree(0),
		status(UNDISCOVERED),
		dTime(-1),
		fTime(-1),
		parent(-1),
		priority(INT_MAX) {

		}
};

template <typename Te>
struct Edge {
	Te data;
	int weight;
	EType type;
	Edge(Te const& d, int w):
		data(d),
		weight(w),
		type(UNDETEMINED) {

		}
};

template <typename Tv, typename Te>
class GraphMatrix : public Graph<Tv, Te> {
private:
	vector<Vertex<Tv> > V; //顶点集合
	vector<vector<Edge<Te>* > > E; //边集合，第i个顶点对应的边集合，E[0]：代表第0个顶点对应的边集合
									//E[0][1]代表第0个顶点到第1个顶点的边

public:
	using Graph<Tv, Te>::n;
	using Graph<Tv, Te>::e;
	GraphMatrix() { 
		//Graph<Tv, Te>::n = Graph<Tv, Te>::e = 0;
		n = e = 0;
	}

	~GraphMatrix() {
		for (int j = 0; j < n; j++) { //删除所有动态创建的边记录
			for (int k = 0; k < n; k++) {
				delete E[j][k];
			}
		}

	}

	virtual Tv& vertex(int i) {
		return V[i].data;
	}

	virtual int inDegree(int i) {
		return V[i].inDegree;
	}

	virtual int outDegree(int i) {
		return V[i].outDegree;
	}

	virtual int firstNbr(int i) {
		return nextNbr(i, n);
	}

	virtual int nextNbr(int i, int j) {
		while ((-1 < j) && (!exists(i, --j)) );

		return j;
	} 

	virtual VStatus& status(int i) {
		return V[i].status;
	}

	virtual int& dTime(int i) {
		return V[i].dTime;
	}

	virtual int& fTime(int i) {
		return V[i].fTime;
	}

	virtual int& parent(int i) {
		return V[i].parent;
	}

	virtual int& priority(int i) {
		return V[i].priority;
	}

	// 插入顶点
	virtual void insert(Tv const& vertex) {
		for (int j = 0; j < n; ++j) {
			E[j].push_back(NULL); //为原先各个顶点预留一条潜在的关联边
		}
		n++; // 顶点数 + 1（计入插入的新顶点）

		E.push_back(std::vector<Edge<Te> *>(n, (Edge<Te> *)NULL)); //为新顶点创建对应的边向量

		return V.push_back(Vertex<Tv>(vertex)); // 顶点向量增加新顶点
	}

	//删除第i个顶点
	virtual Tv remove(int i) {
		for (int j = 0; j < n; ++j) {
			if (exists(i, j)) {//删除i的所有出边
				delete E[i][j]; //删除i->j的边
				V[j].inDegree--; //i->j,即顶点j的入度-1
			}

			if (exists(j, i)) { //删除i的所有入边
				delete E[j][i]; 
				V[j].outDegree--; //j->i,即顶点j的出度-1
			}
		}

		typename vector<vector<Edge<Te>* > >::iterator itEV = E.begin();
		E.erase(itEV + i);//删除第i个顶点对应的边集合
		n--; //更新顶点数

		Tv vBak = vertex(i); //获取第i个顶点的数据，作为返回值

		typename vector<Vertex<Tv> >::iterator itV = V.begin();
		V.erase(itV + i);//删除第i个顶点的数据

		return vBak;
	}

	//边的操作

	//边(i,j)是否存在
	virtual bool exists(int i, int j) {
		return (0 <= i) && (i < n) && (0 <=j ) && (j < n) && E[i][j] != NULL;
	}

	//边(i,j)的类型
	virtual EType& type(int i, int j) {
		return E[i][j]->type;
	}

	//边(i,j)的数据
	virtual Te& edge(int i, int j) {
		return E[i][j]->data;
	}

	virtual int& weight(int i, int j) {
		return E[i][j]->weight;
	}


	virtual void insert(Te const& edge, int w, int i, int j) {
		if (exists(i, j)) { //如果e(i, j)已经存在，则退出
			return;
		}

		E[i][j] = new Edge<Te> (edge, w);
		e++;
		V[i].outDegree++;
		V[j].inDegree++;
	}

	virtual Te remove(int i, int j) {
		Te eBak = edge(i, j); //获取边(i,j)的数据
		delete E[i][j];
		E[i][j] = NULL;
		e--;
		V[i].outDegree--;
		V[j].inDegree--;

		return eBak;
	}
};


template <typename Tv, typename Te>
struct PrimPU { //针对Prim算法的顶点优先级更新器
   virtual void operator() ( Graph<Tv, Te>* g, int uk, int v ) {
      if ( UNDISCOVERED == g->status ( v ) ) //对于uk每一尚未被发现的邻接顶点v
         if ( g->priority ( v ) > g->weight ( uk, v ) ) { //按Prim策略做松弛
            g->priority ( v ) = g->weight ( uk, v ); //更新优先级（数）
            g->parent ( v ) = uk; //更新父节点
         }
   }
};

int main() {
	GraphMatrix<char, int> g;

	g.insert('A'); //0
	g.insert('S'); //1
	g.insert('E'); //2
	g.insert('D'); //3
	g.insert('C'); //4
	g.insert('F'); //5
	g.insert('B'); //6
	g.insert('G'); //7

	g.insert(1, 1, 0, 2);
	g.insert(1, 1, 1, 0);
	
	g.insert(1, 1, 1, 4);
	g.insert(1, 1, 1, 3);
	g.insert(1, 1, 2, 5);
	g.insert(1, 1, 2, 7);
	g.insert(1, 1, 3, 6);
	g.insert(1, 1, 4, 6);
	g.insert(1, 1, 7, 6);
	g.insert(1, 1, 7, 5);

	g.bfs(1);
	printf("\n");

	g.dfs(1);
	printf("\n");

	stack<char> *pvs = g.tSort(1);

	while (!pvs->empty()) {
		printf("%c ", pvs->top());
		pvs->pop();
	}

	if (NULL != pvs) {
		delete pvs;
	}

	printf("\n");

	g.bcc(1);

}