#include <vector>
#include <stdio.h>
using namespace std;

template <typename T>
struct BTNode { //B树节点模板
	BTNode<T> * parent;
	vector<T> key; //关键码向量
	vector<BTNode<T> *> child; //孩子向量，其长度比key多1

	BTNode() { //只能作为根节点构造，初始时有0个关键码和1个空孩子
		parent = NULL;
		child.push_back(NULL);
	}

	//作为根节点，初始化时有1个关键码，2个孩子
	BTNode(T e, BTNode<T> * lc = NULL, BTNode<T> * rc = NULL) {
		parent = NULL;//作为根节点
		key.push_back(e);
		child.push_back(lc);
		child.push_back(rc);  

		if (lc) {
			lc->parent = this;
		}

		if (rc) {
			rc->parent = this;
		}
	}
	
};

template <typename T>
class BTree {
protected:
	int _size;
	int _order;
	BTNode<T> * _root;
	BTNode<T> * _hot;
	void solveOverflow(BTNode<T> *);
	void solveUnderflow(BTNode<T> *);

public:
	BTree(int order = 3):
		_order(order), 
		_size(0) {
			_root = new(std::nothrow) BTNode<T>();
		}

	~BTree() {
		if (_root) {
			delete _root;
			_root = NULL;
		}
	}

	int const order() {
		return _order;
	}

	int const size() {
		return _size;
	}

	BTNode<T> * & root() {
		return _root;
	}

	bool empty() const {
		return !_root;
	}

	BTNode<T> * search(const T& e);
	bool insert(const T& e);
	bool remove(const T& e);
};

//在A向量中查找关键码e，返回不大于e的最大关键码索引，如果没有
template <typename T>
static int findkey(const vector<T> &A, T const & e) {
	size_t i = 0;
	for (i = 0; (i < A.size()) && (A[i] <= e); ++i) { //注意这里(i < A.size())要要写前面，不然如果A.size()为0，那么A[i]就会崩溃
		;
	}

	if (i < A.size()) {
		if (A[i] == e) {
			return i;
		} else { //这里A[i] > e
			return i - 1; //返回不大于e的最大关键码的索引。
		}
	} 

	return -1; //这里A向量里的元素全部大于e，此时返回索引-1，表示没有不大于e的最大关键码。
}

template <typename T>
BTNode<T> * BTree<T>::search(const T& e) {
	BTNode<T> * v = _root;
	_hot = NULL;


	while (v) {
		int r = findkey(v->key, e);
		if (r != -1) { //找到了
			return v;
		}

		_hot = v;
		v = v->child[r + 1]; //没有找到，转入节点e的右子树(因为返回的r上的元素是不大于e的最大关键码的位置)，
							//进行查找
	}

	return NULL;
}

//将关键码e插入B树中
//首先调用search()接口在树中查找该关键码，若查找成功，则按照“禁止重复关键码”的约定直接返回，
//否则查找过程必然终止于某一外部节点v，且其父节点有_hot指示，
//且此时的_hot必然指向某一个叶节点（可能同时也是根节点）。接下来，在该超级叶节点中再次查找关键码，
//尽管必定失败，但却可以确定关键码应该被插入的位置（向量索引），最后只需在这位置插入关键码。
template <typename T>
bool BTree<T>::insert(const T& e) {
	BTNode<T>  * v = search(e);//首先寻找关键码，如果找到就退出
	if (v) {
		return false;
	}

	int r = findkey(_hot->key, e);//查找关键码e应该被插入的位置
	_hot->key.insert(_hot->key.begin() + r + 1, e); //插入关键码e，这里插入的位置比查询的大1，因为查询得到的是不大于关键码的最大关键码的位置
	_hot->child.insert(_hot->child.begin() + r + 2, NULL); //插入关键码e的右孩子指针。右孩子的位置对应于是关键码e的位置 + 1
	_size++; 
	solveOverflow(_hot); //如有必要（若发生上溢），需做分裂

	return true;
}

template <typename T>
void BTree<T>::solveOverflow(BTNode<T> * v) {
	if (_order >= v->child.size()) { //分支数满足条件，没有上溢，直接返回，递归基。
		return;
	}

	int s = _order/2; //此时_order = key.size() = child.size() - 1

	BTNode<T> * u = new(std::nothrow) BTNode<T>();

	// _order -s - 1 代表s右边的元素个数，用总的个数减去s，再减1是因为s是索引，要多减去1（这样才是个数）
	// 个数 = 索引 + 1
	for (int j = 0; j < _order - s - 1; ++j) {//注意这里遍历了s右边元素的个数，因此还有最右边一个child元素没有遍历
		u->child.insert(u->child.begin() + j, v->child[s + 1]);
		v->child.erase(v->child.begin() + s + 1);

		u->key.insert(u->key.begin() + j, v->key[s + 1]);
		v->key.erase(v->key.begin() + s + 1);
	}


	u->child[_order - s - 1] = v->child[s + 1];//将v中最右边的孩子赋值到u，注意这里虽然索引和上面一样都是s+1
							//但是因为上面的已经erase了，child的大小变小了（大小为s+1+1，第一个加1是因为s是索引，大小就需要
	//在索引的基础上+1，第二个加1，是因为child大小会比key的大小大1），所以这里的s+1元素是最右边的孩子
	v->child.erase(v->child.begin() + s + 1);

	if (u->child[0]) { //若u的孩子们非空，将它们的父节点都指向u（相互连接）
		for (int j = 0; j < _order - s; ++j) {
			u->child[j]->parent = u;
		}
	}

	BTNode<T> * p = v->parent;
	if (!p) {
		_root = p = new(std::nothrow) BTNode<T>();
		p->child[0] = v;
		v->parent = p;
	}


	int r = 1 + findkey(p->key, v->key[0]);//这里findkey返回的不大于v->key[0]的最大关键码的索引，
			//即r将就应该插入关键码v->key[s]的索引
	p->key.insert(p->key.begin() + r, v->key[s]);//轴点关键码上升一层
	v->key.erase(v->key.begin() + s);//去掉溢出层的轴点关键码

	p->child.insert(p->child.begin() + r + 1, u); //新结点u和父节点p互联
	u->parent = p;

	solveOverflow(p); //因为轴点关键码上升一层，有可能再次导致溢出，这里递归处理（最多O(logn)次）
}


template <typename T>
bool BTree<T>::remove(const T& e) {
	BTNode<T> * v = search(e); //关键码若查找不到，就直接退出
	if (!v) {
		return false;
	}

	int r = findkey(v->key, e); //在关键码所在的超级节点中查找关键码e的索引
	if (v->child[0]) { //若v非叶子，则e的后继必属于其右子树中
		BTNode<T> * u = v->child[r + 1]; //e的后继，在v的右子树中
		while (u->child[0]) { //寻找e的直接后继，e的后继在v的右子树中，之后在u第一个孩子下（即是最左边的孩子是比e大，
				//且是比e大的当中最小的那个,这个才是e的直接后继，且直接后继是叶节点）
			u = u->child[0];
		}

		v->key[r] = u->key[0]; //交换关键码e和它的直接后继,至此，节点v已经把e删除，使用直接后继的关键码代替了e的值
		v = u; //这里就是将u节点赋值给v变量。来进行删除e的直接后继（e的直接后继已经上移到v，u中需要删除，这里不会原有已经删除的e的
		//v节点，因为v是指针，所以不管怎么对v赋值，不会改变之前v的东西），
		//之所以这里要用v来操作，是因为这里if语句之外，用的是v变量进行操作，如果直接用u进行操作删除，那么下面的出了if
		//的句子中也应该用u，这样就没法把v是叶节点和不是节点的2中情况统一编写代码。
		r = 0; //e的直接后继是最左边的关键码，所以索引是0
	}

	v->key.erase(v->key.begin() + r);
	v->child.erase(v->child.begin() + r + 1);//删除e的直接后继的右孩子，因为左孩子是第一个，不能删除，依然要作为u节点的第一个孩子
	_size--;

	solveUnderflow(v);

	return true;
}

template <typename T>
void BTree<T>::solveUnderflow(BTNode<T> * v) {
	if ((_order + 1) / 2 <= v->child.size()) { //节点分支树 >= (m/2)的天花板值，就合法,递归基
		return;
	}
	
	BTNode<T> * p = v->parent; //p为v的父亲
	if (!p) { //v已经是根节点，且v中没有关键码，但包含唯一的非空孩子，此时，就将该非空孩子
		if (!v->key.size() && v->child[0]) { //代替根节点，并将根节点的父亲和孩子都置为NULL
			_root = v->child[0];
			_root->parent = NULL;
			v->child[0] = NULL;

			if (v) { //v已经不需要了，释放资源，树的整体高度降低一层
				delete v;
				v = NULL;
			}	
		}
	}

	int r = 0;
	while (p->child[r] != v) { //找出v是p的第几个孩子，因为此时v可能不包含关键码，所以不能通过关键码查找
		r++;
	}

	//情况1 向左兄弟借关键码
	if (0 < r) {//v不是p的第一个孩子
		BTNode<T> * ls = p->child[r - 1]; //ls为v的左兄弟，这里左兄弟必然存在（r - 1 >= 0)
		if ((_order + 1) / 2 < ls->child.size()) { //左兄弟有足够的关键码借 （至少大于等于（m/2)的天花板个， 
						//这里没有等号是因为和ls的child数量比较，child的数量比key的数量要多1）
			v->key.insert(v->key.begin(), p->key[r - 1]); //将父亲p中借一个节点给v，作为最小关键码（r-1的关键码就是恰好是v中最小）
			p->key[r - 1] = ls->key[ls->key.size() - 1]; //将左兄弟中的最大关键码关键码借给父亲p，
			ls->key.erase(ls->key.end() - 1); //删除左兄弟借过的关键码

			//将ls的最右侧的孩子过继给v
			v->child.insert(v->child.begin(), ls->child[ls->child.size() - 1]); //将v中借过来的关键码对应的左孩子更新
			ls->child.erase(ls->child.end() - 1); //删除左兄弟中借出去的关键码的右孩子

			if (v->child[0]) { //如果v有最左的孩子（这个孩子就是从ls中借过来的），就需要互联父亲。
				v->child[0]->parent = v;
			}

			return;
		}
	}

	// 情况2：向右兄弟借关键码

	if (p->child.size() - 1 > r) {
		BTNode<T> * rs = p->child[r + 1];
		if ((_order + 1) / 2 < rs->child.size()) { //右兄弟有足够的关键码可借
			v->key.insert(v->key.end() - 1, p->key[r]); //p借出关键码给v，作为最大关键码
			p->key[r] = rs->key[0]; //将rs的最小关键码替换直接接出去的关键码
			rs->key.erase(rs->key.begin()); //删除rs中借出去的关键码

			//将rs的最左侧的孩子过继给v
			v->child.insert(v->child.end() - 1, rs->child[0]); //将v中借过来的关键码对应的右孩子更新
			rs->child.erase(rs->child.begin()); //删除右兄弟中借出去的关键码的左孩子

			if (v->child[v->child.size() - 1]) {
				v->child[v->child.size() - 1]->parent = v;
			}

			return;
		}
	}

	//情况3：左或右兄弟要么为空（但不可能同时），要么都关键码不足-----合并
	if (0 < r) {
		BTNode<T> * ls = p->child[r - 1]; //ls为v的左兄弟，这里必然存在
		ls->key.insert(ls->key.end() - 1, p->key[r - 1]); //将p的第r-1的关键码转入ls中
		p->key.erase(p->key.begin() + r - 1); //删除p中的第r-1个关键码
		p->child.erase(p->child.begin() + r); //删除p中第r个孩子，v不在是p的第r个孩子

		ls->child.insert(ls->child.end() - 1, v->child[0]);//将v的第一个孩子插入到ls的孩子的最后，合并孩子
		v->child.erase(v->child.begin());//删除v中第一个孩子，因为它已经合并到ls中去了

		if (ls->child[ls->child.size() - 1]) { //因为ls的最右侧孩子是从v中过继来的，如果最右侧孩子存在，那就要更新父亲
			ls->child[ls->child.size() - 1]->parent = ls;
		}

		//将v中的关键码和孩子依次转入ls中
		while (!v->key.empty()) {
			ls->key.insert(ls->key.end() - 1, v->key[0]);
			v->key.erase(v->key.begin());
			ls->child.insert(ls->child.end() - 1, v->child[0]);
			v->child.erase(v->child.begin());

			if (ls->child[ls->child.size() - 1]) {
				ls->child[ls->child.size() - 1]->parent = ls;
			}
		}

		if (v) {
			delete v;
			v = NULL;
		}

	} else { //与左兄弟合并
		BTNode<T> * rs = p->child[r + 1];
		rs->key.insert(rs->key.begin(), p->key[r]);
		p->key.erase(p->key.begin() + r);
		p->child.erase(p->child.begin() + r);

		rs->child.insert(rs->child.begin(), v->child[v->child.size() - 1]);
		v->child.erase(v->child.end() - 1);

		if (rs->child[0]) {
			rs->child[0]->parent = rs;
		}

		while (!v->key.empty()) {
			rs->key.insert(rs->key.begin(), v->key[v->key.size() - 1]);
			v->key.erase(v->key.end() - 1);

			rs->child.insert(rs->child.begin(), v->child[size() - 1]);
			v->child.erase(v->child.end() - 1);

			if (rs->child[0]) {
				rs->child[0]->parent = rs;
			}
		}

		if (v) {
			delete v;
			v = NULL;
		}

	}

	solveUnderflow(p); //上升一层，如有必要则继续分裂-----至多递归O(logn)层

	return;
}

template <typename T>
struct VisitPrint { //打印节点数据的仿函数
	void operator() (T &e) const {
	//	cout << e << ' ';
		printf("%d " , e);
	}
};

//test
template <typename T>
void testBinTree () {
	BTree<T> bt(4); //4阶B树
	bt.insert(36);
    bt.insert(27);
    bt.insert(6);
    bt.insert(1);
	bt.insert(58);
	bt.insert(53);
	bt.insert(64);
	bt.insert(40);
	bt.insert(46);


	//bt.travIn(VisitPrint<T>());
	//printf("\n");

	///bt.travPost(VisitPrint<T>());
	//printf("\n");

	//bt.travPre(VisitPrint<T>());
	//printf("\n");

	bt.remove(27);
	//bt.travIn(VisitPrint<T>());
	//printf("\n");
}


int main() {

	BTNode<int> c;
	VisitPrint<int> v;

	testBinTree<int>();

	return 0;
}
