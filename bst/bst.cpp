#include "bintree.h"

//不考虑有相同元素
template <typename T>
class BST : public BinTree<T> {
protected:
	BinNode<T> * _hot; //命中节点的父亲
	//BinNode<T> connect34 
public:
	virtual BinNode<T>* & search(const T& e);
	virtual BinNode<T> * insert(const T& e);
	virtual bool remove(const T& e);
};

//查找算法：采用减而治之的思路与策略，其执行过程为：从树根出发，逐步地缩小查找范围，直到发现目标（成功）或缩小至空树（失败）
//在以v为根的BST子树中查找关键码e
//时间复杂度：在最坏情况下为O(n)，退化为一维序列表，此时查找就是顺序查找
template <typename T>
static BinNode<T>* & searchIn(BinNode<T>* &v, const T& e, BinNode<T>* & hot) {
	if (!v || (e == v->data)) {//递归基，没找到返回NULL，找到则返回v
		return v;
	}

	hot = v; //当前节点，也是下次查找的父节点
	return searchIn(((e < v->data) ? v->lc : v->rc), e, hot);
}

//在BST中查找关键码e
template <typename T>
BinNode<T>* & BST<T>::search(const T& e) {
	return searchIn(this->_root, e, _hot = NULL);
}

//插入算法：首先查找确定插入的位置，这里如果原来就有关键码e将直接返回，
//否则返回NULL，并将指向NULL指针赋值为新结点的指针。
//时间复杂度：时间主要消耗在search()和updateHeightAbove()的调用。时间复杂度取决于新结点的深度，在坏情况为不超过全树的高度。
template <typename T>
BinNode<T>* BST<T>::insert(const T& e) {
	BinNode<T>* & x = search(e);//确认目标是否存在，存在就直接返回
	if (x) {
		return x;
	}

	printf("root %p\n", this->_root);
	x = new(std::nothrow) BinNode<T> (e, _hot); //创建新结点x，以e为关键码，_hot为父亲，这样x就指向了新结点，即插入成功
	if (NULL == x) {
		return NULL;
	}

	this->_size++; //更新规模
	printf("root %p\n", this->_root);
	this->updateHeightAbove(x); //更新x及历代祖先的高度
	printf("root %p\n", this->_root);
	return x;

}
template <typename T>
bool BST<T>::remove(const T& e) {
	BinNode<T> * & x = search(e);
	if (!x) {
		return false;
	}

	removeAt(x, _hot);
	this->_size--;
	this->updateHeightAbove(_hot);

	return true;
}


//删除算法：首先查找，判断目标是否存在于树中，若存在，返回其位置，然后进行具体的删除操作。
//1. 单分之情况（目标结点只有一个孩子或者没有孩子）
//这种情况只需要将其孩子替换为目标节点，之后更新全树的规模，释放被删除的节点，并自下而上逐个更新替代节点（其孩子）
//历代祖先的高度，即首个节点为_hot指向的节点。

//2. 双分支情况
//调用succ()算法，找出该节点的直接后继，交换两者的数据项，然后将后继节点等效的看做待删除ed目标，不难验证，该后继
//节点必无左儿子，从而转化为情况1的做法

//时间复杂度：主要消耗在search()、succ()、updataHeightAbove()的调用，在树的任一高度，至多消耗O(1),所以总体复杂度
//不超过全树的高度O(h)， 目前最坏情况为O(n)
template <typename T>
static BinNode<T> *  	removeAt(BinNode<T> * & x, BinNode<T> * & hot) {
	BinNode<T> * w = x; //实际被删除的节点x先复制到w
	BinNode<T> * succ = NULL; //w的替换着

	if (!HasLChild(*x)) { //如果*x没有左儿子，就将x替换为右儿子,这里右儿子可能为NULL
		succ = x = x->rc;
	}  else if (!HasRChild(*x)) { //若*x没有右孩子，就将x替换为左儿子，这里左儿子是不可能为空（左儿子是否为空第一个if已判断）
		succ = x = x->lc; // succ != NULL
	} else { // 若左右孩子均存在，则选择x的直接后继替换实际被摘除节点的位置，这里只需要交换x（删除节点）和w（x的直接后继）的元素数据即可。
		w = w->succ();
		swap(x->data, w->data);

		BinNode<T> * u = w->parent; //u为替换者的父亲（此时替换者的数据已经是删除节点的数据了）
		((u == x) ? u->rc : u->lc) = succ = w->rc; //w（直接后继）肯定没有左孩子，但是右孩子可能有,注意这里将
						//w的右孩子连接到u的左孩子或右孩子位置处，这样w就被隔离了（w的父亲不再是u）
	}

	hot = w->parent; //记录实际被删除节点的父亲
	if (succ) { //如果w的替换着不为空，那么就将其父亲与hot关联，因为hot是命中节点的父亲
		succ->parent = hot;
	}

	if (w) { //释放被摘除节点
		delete w;
		w = NULL;
	}

	return succ;
}




//test
template <typename T>
void testBinTree () {
	BST<T> bt;
	BinNode<T> * r = bt.insert(36);
#if 1
	BinNode<T> * n1 = bt.insert(27);
	BinNode<T> * n2 = bt.insert( 6);
	BinNode<T> * n3 = bt.insert(58);
	BinNode<T> * n4 = bt.insert(53);
	BinNode<T> * n5 = bt.insert(64);
	BinNode<T> * n6 = bt.insert(40);
	BinNode<T> * n7 = bt.insert(46);


	bt.travIn(VisitPrint<T>());
	printf("\n");

	bt.remove(36);
	bt.travIn(VisitPrint<T>());
	printf("\n");
#endif
}


int main() {

	BinNode<int> c;
	VisitPrint<int> v;

	testBinTree<int>();

	return 0;
}
