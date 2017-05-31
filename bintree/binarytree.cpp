#include <iostream>
#include <stack>
#include <queue>
#include <stdio.h>
using namespace std;

#define IsRoot(x) (!((x).parent))
#define IsLChild(x) ( !IsRoot(x) && (&(x) == (x).parent->lc))
#define IsRChild(x) ( !IsRoot(x) && (&(x) == (x).parent->rc))
#define HasParen(x) (!IsRoot(x))
#define HasLChild(x) ((x).lc)
#define HasRChild(x) ((x).rc)
#define FromParentTo(x) (IsRoot(x) ? _root : (IsLChild(x) ? (x).parent->lc : (x).parent->rc)) //转化为指针，通过父节点获取

#define stature(p)  ((p) ? (p)->height: -1) //节点高度，空树的高度为-1
#define max(a,b) (((a) > (b)) ? (a) : (b));
#define SALF_DELETE(p) do {if (p) { delete p; p = NULL;}} while(0);

template <typename T>
struct VisitPrint { //打印节点数据的仿函数
	void operator() (T &e) const {
	//	cout << e << ' ';
		printf("%c " , e);
	}
};

template <typename T>
static bool lt(T *a, T *b) {
	return lt(*a, *b);
}

template <typename T>
static bool lt(T &a, T &b) {
	return a < b;
}

template <typename T>
struct BinNode {
	T data;
	BinNode<T> * parent;
	BinNode<T> * lc;
	BinNode<T> * rc;
	int height;

	BinNode() : parent(NULL),
				lc(NULL),
				rc(NULL),
				height(0) { }

	BinNode(T e, BinNode<T> * parent = NULL, BinNode<T> * lc = NULL, BinNode<T> * rc = NULL, int height = 0):
			data(e),
			parent(parent),
			lc(lc),
			rc(rc),
			height(height) { }

	int size(); //当前节点及后代的总数
	BinNode<T> * insertAsLC(T const&); //作为当前节点的左孩子插入新节点
	BinNode<T> * insertAsRC(T const&); //作为当前节点的右孩子插入新节点
	BinNode<T> * succ(); //中序遍历规则中的直接后继

	template <typename VST>
	void travLevel(const VST&); //层次遍历

	template <typename VST>
	void travPre(const VST&); // 先序遍历

	template <typename VST>
	void travIn(const VST&); //中序遍历

	template <typename VST>
	void travPost(const VST&); //后续遍历

	bool operator < (BinNode const& bn) {
		return data < bn.data;
	}

	bool operator == (BinNode const& bn) {
		return data == bn.data;
	}			
};

template <typename T>
int BinNode<T>::size() {
	int s = 1; //计入本身
	if (lc) {
		s += lc->size(); //递归计入左子树大小
	}

	if (rc) {
		s += rc->size(); //递归计入右子数大小
	}
	return s;
}

template <typename T>
BinNode<T> * BinNode<T>::insertAsLC(T const& e)  {
	return lc = new(std::nothrow) BinNode(e, this);//把e当作当前节点的左孩子数据并插入，注意这里默认插入前是没有左孩子的，不然会被覆盖
}

template <typename T>
BinNode<T> * BinNode<T>::insertAsRC(T const& e)  {
	return rc = new(std::nothrow) BinNode(e, this);//把e当作当前节点的右孩子数据并插入
}

template <typename T>
BinNode<T> * BinNode<T>::succ() {//定位当前节点的直接后继（中序遍历规则，左中右）
	BinNode<T> * s = this;	// 记录后继的临时变量
	if (rc) { //如果有右孩子，则直接后继肯定在右子树中
		s = rc;

		while (HasLChild(*s) ) {// 直接后继在右子树中的最靠左边（左边且深度最大）的结点
			s = s->lc;
		}
	} else { //如果没有右孩子，直接后继肯定在将当前节点包含于其左子树中的最低祖先
		while (IsRChild(*s)) { // 如果当前节点作为右孩子，需要不断向左上方移动
			s = s->parent;
		}
		s = s->parent; // 最后在往右上方移动一步，就是直接后继
	}

	return s;
}

template <typename T, typename VST>
void travPre_R(BinNode<T> * x, const VST& visit) {// 先序遍历递归版（中左右），这个可以看作是尾递归
	if (!x) {
		return;
	}
	visit(x->data);
	travPre_R(x->lc, visit);
	travPre_R(x->rc, visit);	
}

template <typename T, typename VST>
void travPre_I1(BinNode<T> * x, const VST& visit) {//迭代版本1，是将上述尾递归修改为迭代
	stack<BinNode<T>* > s;
	if (x) {
		s.push(x); //当前节点入栈（当前遍历树的根）
	}

	while (!s.empty()){ 
		x = s.top();
		s.pop();
		visit(x->data); //访问元素，因为中左右，即先访问
		
		if (HasRChild(*x)) { // 这里先入栈右孩子，再入栈左孩子，这样弹出的时候就符合中左右的顺序
			s.push(x->rc);
		}

		if (HasLChild(*x)) {
			s.push(x->lc);
		}
	}
}

//从当前节点出发，沿左分支不断深入到没有左分支的节点，且沿途中的节点立即访问
template <typename T, typename VST>
static void visitAlongLeftBranch(BinNode<T> *x, const VST& visit, stack<BinNode<T> *> &s) {
	while (x) {
		visit(x->data); //访问当前节点
		s.push(x->rc); //这里右孩子入栈，因为深入到最左边节点后，回溯的时候，就从最左边节点的右子树再进行visitAlongLeftBranch
		x = x->lc; // 沿左分支深入一层
	}	
}

template <typename T, typename VST>
void travPre_I2(BinNode<T> * x, const VST& visit) {//迭代版本2
	stack<BinNode<T> *> s;
	while(1) {
		visitAlongLeftBranch(x, visit, s); //从当前节点出发到最左的节点（注意后面会该节点的右子树进行同样的遍历，即批次访问）
		
		if (s.empty()) {
			break;
		}

		x = s.top(); //弹出下一批的起点
		s.pop();
	}
}

template <typename T, typename VST>
void travIn_R(BinNode<T> * x, const VST& visit) {// 中序遍历递归版（左中右）
	if (!x) {
		return;
	}
	travIn_R(x->lc, visit);
	visit(x->data);
	travIn_R(x->rc, visit);	
}

// 从当前节点出发，沿左分支不断深入，直到没有左分支的节点（注意这里不能先访问，而是先保存在栈中，左中右）
template <typename T>
static void goAlongLeftBranch(BinNode<T> *x, stack<BinNode<T> *> &s) {
	while (x) {
		s.push(x); //将沿路节点保存在栈中，栈的大小正比于二叉树的高度
		x = x->lc; 
	}
}

template <typename T, typename VST>
void travIn_I1(BinNode<T> * x, const VST& visit) {//迭代版本1
	stack<BinNode<T> *> s;
	while(1) {
		goAlongLeftBranch(x, s); //当前节点出发，直到最左边的节点，沿路节点入栈，之后对于该最深处的左节点的右子树进行同样处理
		
		if (s.empty()) { //如果栈为空，即所有节点处理完成，之后退出。
			break; 
		}
		x = s.top(); //弹出栈顶节点，访问
		s.pop();

		visit(x->data);
		x = x->rc; //转向右子树
	}
}

template <typename T, typename VST>
void travIn_I2(BinNode<T> * x, const VST& visit) {//迭代版2，对迭代1版本的改写
	stack<BinNode<T> *> s; //栈的大小正比于二叉树的高度
	while (1) {
		if (x) {
			s.push(x); //沿路节点入栈
			x = x->lc; //深入左子树
		} else if (!s.empty()) {
			x = s.top(); //弹出栈顶元素访问
			s.pop();
			visit(x->data);
			x = x->rc; //转向右子树
		} else {
			break;
		}
	}
}

template <typename T, typename VST>
void travIn_I3(BinNode<T> * x, const VST& visit) {//迭代版3，无辅助栈
	bool backtrace = false; //前一步是否刚从右子树回溯（省去栈），仅O(1)的辅助空间

	while (1) {
		if (!backtrace && HasLChild(*x)) {// 若有左子树且不是刚刚回溯，则深入遍历左子树
			x = x->lc;
		} else { //无左子树或刚刚回溯
			visit(x->data);

			if (HasRChild(*x)) {//若有右子树，就深入右子树比遍历，并关闭回溯标志
				x = x->rc;
				backtrace = false;
			} else { // 右子树为空，就需要回溯
				if (!(x = x->succ())) { // x赋值为它的直接后继，如果为NULL就代表处理结束，退出。
					break;
				}
				backtrace = true;
			}
		}

	}
}

template <typename T, typename VST>
void travIn_I4(BinNode<T> *x, const VST& visit) {//迭代版4
	while (1) {
		if (HasLChild(*x)) {// 若有左子树，则深入遍历左子树
			x = x->lc;
		} else {
			visit(x->data);

			while (!HasRChild(*x)) { // 如果没有右孩子就需要回溯至直接后继，直接后继为空就退出。
				if (!(x = x->succ())) {
					return;
				} else { //直接后继不为空，就访问
					visit(x->data);
				}
			}
			x = x->rc; // 出了上面的while循环就代表有右孩子，转向非空的右子树
		}

	}
}

template <typename T, typename VST> // 后序遍历递归版（左右中）
void travPost_R(BinNode<T> * x, const VST& visit) {
	if (!x) {
		return;
	}
	travPost_R(x->lc, visit);
	travPost_R(x->rc, visit);	
	visit(x->data);
}

//在以栈s中的栈顶为根的子树中，找到最高左侧可见叶节点（就是靠左最深入的节点，该节点可能是左孩子也可能是右孩子）
template <typename T>
static void gotoHLVFL(stack<BinNode<T> *> &s) {
	BinNode<T> *x = NULL;
	while ( x = s.top()) { // 自顶向下，反复检查当前节点, 注意这里是将栈顶节点赋值给x
		if (HasLChild(*x)) { // 尽可能向左
			if (HasRChild(*x)) { //若有右孩子，优先入栈（左右中）
				s.push(x->rc);
			}
			s.push(x->lc); //再将左孩子入栈
		} else  {
			s.push(x->rc); // 没有左孩子，才将右孩子入栈，注意进入到这里说明x没有左孩子，因此对于最后一个节点(既没有左孩子也没有右孩子)，这里栈就存储了NULL节点
		}           //, 也可以更改如下注释的实现
	}
	s.pop(); // 弹出最后入栈的栈顶空节点
}

# if 0
template <typename T>
static void gotoHLVFL(stack<BinNode<T> *> &s) {
	BinNode<T> *x = NULL;
	while ( x = s.top()) { 
		if (HasLChild(*x)) { 
			if (HasRChild(*x)) { 
				s.push(x->rc);
			}
			s.push(x->lc); 
		} else if (HasRChild(*x)) {
			s.push(x->rc); 
		} else {           
			break;
		}
	}
}
#endif

template <typename T, typename VST>
void travPost_I(BinNode<T> *x, const VST& visit) { // 后续遍历迭代版 （左右中）
	stack<BinNode<T> *> s;
	if (x) {
		s.push(x); //根节点入栈
	}

	while (!s.empty()) {
		if (s.top() != x->parent) { //这里如果栈顶节点不是上一次弹出节点的父亲（那么就必为上一次弹出节点的右兄弟，第一次根节点的父亲是NULL，不属于右兄弟）
			gotoHLVFL(s);  // ，就需要以其右兄弟为根的子树中，继续寻找HLVFL（相当于递归深入其中）
		}
		x = s.top(); //这里就是上次节点的直接后继（左右中）
		s.pop();

		visit(x->data);
	}
}

// 广度优先遍历或层次遍历
template <typename T>
template <typename VST>
void BinNode<T>::travLevel(const VST& visit) {
	queue<BinNode<T> *> q;
	q.push(this); //根节点入队

	while (!q.empty()) {
		BinNode<T> *x = q.front(); //取队首节点
		q.pop();
		visit(x->data);

		if (HasLChild(*x)) { //左孩子入队
			q.push(x->lc);
		}

		if (HasRChild(*x)) { //右孩子入队
			q.push(x->rc);
		}
	}	
	return;
}

template <typename T>
template <typename VST>
void BinNode<T>::travPre(const VST& visit) {
#if 0
	travPre_R(this, visit);
#elif 0 
	travPre_I1(this, visit);
#elif 1
	travPre_I2(this, visit);
#endif
}

template <typename T>
template <typename VST>
void BinNode<T>::travIn(const VST& visit) {
#if 0
	travIn_R(this, visit);
#elif 0
	travIn_I1(this, visit);
#elif 0
	travIn_I2(this, visit);
#elif 0
	travIn_I3(this, visit);
#elif 1
	travIn_I4(this, visit);
#endif
}

template <typename T>
template <typename VST>
void BinNode<T>::travPost(const VST& visit) {
#if 0
	travPost_R(this, visit);
#elif 1
	travPost_I(this, visit);
#endif
}


//bintarytree
//二叉树模板类
template <typename T>
class BinTree {
protected:

		int _size; //规模
		BinNode<T> * _root; //根节点
		virtual int updateHeight(BinNode<T> * x); //更新节点x的高度，注意这里是虚函数，因为对于某些二叉树（红黑树），高度定义是不一样的，需要重写
		void updateHeightAbove(BinNode<T> * x); //更新节点x及其祖先的高度，因为x的高度变化后，其祖先的高度也会变化

public:
		BinTree():
			   	_size(0),
			   	_root(NULL) {}

		~BinTree() {
			if (_size > 0) {
				remove(_root);
			}
		}

		int size() const {//规模
				return _size;
		}
		bool empty() const {//判空
			return !_root;
		}

		BinNode<T> * root() const { //根节点
			return _root;
		}

		BinNode<T> * insertAsRoot(T const & e); //e作为根节点插入
		BinNode<T> * insertAsLC(BinNode<T> *x, T const& e); //e作为x的左孩子插入（x原来没有左孩子）
		BinNode<T> * insertAsRC(BinNode<T> *x, T const& e); //e作为x的右孩子插入（x原来没有右孩子）
		BinNode<T> * attachAsLC(BinNode<T> *x, BinTree<T> * &Tree); //Tree作为x左子树接入
		BinNode<T> * attachAsRC(BinNode<T> *x, BinTree<T> * &Tree); //Tree作为x右子树接入

		int remove (BinNode<T> * x); //删除以x为根节点的子树，返回该删除子树的规模
		BinNode<T> * secede(BinNode<T> * x); //删除以x为根节点的子树,并将其转换为一颗独立的子树

		template <typename VST>
		void travLevel(const VST& visit) {
			if (_root) {
				_root->travLevel(visit);
            }
		}

		template <typename VST>
		void travPre(const VST& visit) {
			if (_root) {
				_root->travPre(visit);
            }
		}

		template <typename VST>
		void travIn(const VST& visit) {
			if (_root) {
				_root->travIn(visit);
            }
		}

		template <typename VST>
		void travPost(const VST& visit) {
			if (_root) {
				_root->travPost(visit);
            }
		}

		bool operator < (BinTree<T> const& t) {
			return _root && t._root && lt(_root, t._root);
		}

	    bool operator == (BinTree<T> const& t) {
			return _root && t._root && (_root == t.root);
		}

};



template <typename T>
BinNode<T> * BinTree<T>::insertAsRoot(T const & e) {
	_size = 1;
	return _root = new BinNode<T> (e); // 将e当作根节点插入
}

template <typename T>
BinNode<T> * BinTree<T>::insertAsLC(BinNode<T> *x, T const& e) {
	_size++; // 规模 + 1
	x->insertAsLC(e);
	updateHeightAbove(x); //跟新x及其祖先的高度
	return x->lc; //返回所插入的节点
}

template <typename T>
BinNode<T> * BinTree<T>::insertAsRC(BinNode<T> *x, T const& e) {
	_size++;
	x->insertAsRC(e);
	updateHeightAbove(x);
	return x->rc;
}

template <typename T>
int  BinTree<T>::updateHeight(BinNode<T> * x) {
	return x->height = 1 + max(stature(x->lc), stature(x->rc)); // 1是x本身， 然后加上 左子树和右子树高度的最大值
}

template <typename T>
void BinTree<T>::updateHeightAbove(BinNode<T> * x) {
	while (x) { // 从x出发，直到根节点，遍历x及其祖父
		updateHeight(x);
		x = x->parent;
	}
}

//将Tree作为左子树接入，
template <typename T>
BinNode<T> * BinTree<T>::attachAsLC(BinNode<T> *x, BinTree<T> * &Tree) { //这里Tree用引用就是要更改Tree的值
	if (x->lc = Tree->_root) { //这里先将子树Tree的根赋值给x的左孩子（注意是赋值），判断Tree的根是否有效
		x->lc->parent = x; // ,如果有效，就将x左孩子（Tree子树）的父亲更新为x
	}

	_size += Tree->_size; //更新规模
	updateHeightAbove(x); //更新x及其祖父的高度
	// Tree->_root = NULL; // 注意这了不能delete该子树，因为上面接入的是地址，没有完全copy
	// Tree->_size = 0;
	// SALF_DELETE(Tree);
	//Tree = NULL;

	return x;	
}

template <typename T>
BinNode<T> * BinTree<T>::attachAsRC(BinNode<T> *x, BinTree<T> * &Tree) {
	if (x->rc = Tree->_root) {
		x->rc->parent = x;
	}

	_size += Tree->_size;
	updateHeightAbove(x);
	// 注意这了不能delete该子树，因为上面接入的是地址，没有完全copy

	return x;
}

//删除二叉树中位置x处的节点及后代，返回被删除节点的数量
template <typename T>
static int removeAt(BinNode<T> * x) {
	if (!x) { //递归基准，空树
		return 0;
	}

	int n = 1 + removeAt(x->lc) + removeAt(x->rc); //递归释放左、右子树
	SALF_DELETE(x); // 释放节点

	return n; 
}

template <typename T>
int BinTree<T>::remove (BinNode<T> * x) {
	FromParentTo(*x) = NULL; // 切断来自父节点的指针
	updateHeightAbove(x->parent); //更新祖先的高度
	int n = removeAt(x); // 删除子树x，内部递归实现
	_size -= n; // 更新规模

	return n;	
}

//将子树x从当前树中摘除，并将其封装为一颗棵独立的树
template <typename T>
BinNode<T> * BinTree<T>::secede(BinNode<T> * x) {
	FromParentTo(*x) = NULL; // 切断来自父节点的指针
	updateHeightAbove(x->parent);
	BinTree<T> *S = new(std::nothrow) BinTree<T>; //创建新树
	S->_root = x;
	x->parent = NULL;
	S->_size = x->size();
	_size -= S->_size;

	return S;
}

//test
template <typename T>
void testBinTree () {
	BinTree<T> bt;
	BinNode<T> * r = bt.insertAsRoot('A');
	BinNode<T> * n1 = bt.insertAsLC(r, 'B');
	BinNode<T> * n2 = bt.insertAsLC(n1, 'D');
	BinNode<T> * n3 = bt.insertAsLC(n2, 'H');
	BinNode<T> * n4 = bt.insertAsRC(n1, 'E');


	BinNode<T> * n5 = bt.insertAsRC(r, 'C');
	BinNode<T> * n6 = bt.insertAsLC(n5, 'F');
	BinNode<T> * n7 = bt.insertAsRC(n5, 'G'); 

	bt.travPre(VisitPrint<T>());
	printf("\n");

	bt.travIn(VisitPrint<T>());
	printf("\n");
	
	bt.travPost(VisitPrint<T>());
	printf("\n");

	bt.travLevel(VisitPrint<T>());
	printf("\n");

	cout << r->size() << endl;
	cout << r->height << endl;

	//attach tree
	BinTree<T> bt2;
	BinTree<T> *Pbt2 = &bt2;
	BinNode<T> * r2 = bt2.insertAsRoot('H');
	BinNode<T> * n8 = bt2.insertAsLC(r2, 'I');
	BinNode<T> * n9 = bt2.insertAsLC(n8, 'J');
	bt.attachAsRC(n7, Pbt2); //这里不能直接传&bt2，因为引用是能改变的，不能是右值

	bt.travPre(VisitPrint<T>());
	printf("\n");

	cout << r->size() << endl;
	cout << r->height << endl;
}

int main(int argc, char * argv[]) 
{
	BinNode<char> c;
	VisitPrint<char> v;
		
	testBinTree<char>();
	return 0;
}
