#include "bintree.h"

template <typename T>
struct PQ { //优先级队列PQ模板类
   virtual void insert ( T ) = 0; //按照比较器确定的优先级次序插入词条
   virtual T getMax() = 0; //取出优先级最高的词条
   virtual T delMax() = 0; //删除优先级最高的词条
};

template <typename T>
class PQ_LeftHeap : public PQ<T>, public BinTree<T> {
public:
	PQ_LeftHeap() {}
	PQ_LeftHeap(T * E, int n) {
		for (int i = 0 ; i < n; ++i) { //批量构建，可改进为Floyd建堆算法
			insert(E[i]);
		}
	}

	void insert(T); //按照比较器确定的优先级次序插入词条
	T getMax(); //取出优先级最高的元素
	T delMax(); //删除优先级最高的元素
};

template <typename T>
static BinNode<T> * merge(BinNode<T> * a, BinNode<T> * b) {
	if (!a) { //a不存在的话就返回b
		return b;
	}

	if (!b) { //b不存在的话就返回a
		return a;
	}

	if (lt(a->data, b->data)) { //如果a的优先级比b小的话，就交换a和b，确保a>=b
		swap(a, b);
	}

	a->rc = merge(a->rc, b); // 将a的右子堆和b合并为新堆，并为a的右孩子
	a->rc->parent = a; //a的右孩子（合并后的新堆）连接父亲a

	if (!a->lc || a->lc->npl < a->rc->npl) { //如果a的左孩子是空或者a的左孩子的npl小于右孩子的npl，就需要交换a的左右孩子
		swap(a->lc, a->rc);
	}

	a->npl = a->rc ? a->rc->npl + 1 : 1; //a的npl为a的右孩子的npl+1

	return a;
}

template <typename T>
T PQ_LeftHeap<T>::getMax() {
	return this->_root->data;
}

template <typename T>
void PQ_LeftHeap<T>::insert(T e) {
	BinNode<T> * v = new(std::nothrow) BinNode<T>(e); //创建新结点
	if (NULL == v) {
		return;
	}

	this->_root = merge(this->_root, v); //通过合并完成新结点的插入
	this->_root->parent = NULL; //合并后的新堆的根，其父亲设置为NULL

	this->_size++; //更新规模
}

template <typename T>
T PQ_LeftHeap<T>::delMax() {
	BinNode<T> * lHeap = this->_root->lc;
	BinNode<T> * rHeap = this->_root->rc;

	T e = this->_root->data;
	delete this->_root;
	this->_size--;

	this->_root = merge(lHeap, rHeap);
	if (this->_root) {
		this->_root->parent = NULL;
	}

	return e;
}

int main() {

	int a[] = {17,13,12,6};
	int aLen = sizeof(a)/sizeof(int);
	PQ_LeftHeap<int> aHeap(a, aLen);

	aHeap.travLevel(VisitPrint<int>());
	printf("\n");

	aHeap.travPre(VisitPrint<int>());
	printf("\n");

	aHeap.travIn(VisitPrint<int>());
	printf("\n");

	int b[] = {15,10,8};
	int bLen = sizeof(b)/sizeof(int);
	PQ_LeftHeap<int> bHeap(b, bLen);

	bHeap.travLevel(VisitPrint<int>());
	printf("\n");

	bHeap.travIn(VisitPrint<int>());
	printf("\n");

	BinNode<int> * r = merge(aHeap.root(), bHeap.root());

	r->travLevel(VisitPrint<int>());
	printf("\n");

#if 0
	heap.travLevel(VisitPrint<int>());
	printf("\n");

	heap.insert(100);
	heap.travLevel(VisitPrint<int>());
	printf("\n");
	
	heap.delMax();
	heap.travLevel(VisitPrint<int>());
	printf("\n");
#endif
	cout << endl;

	return 0;
}