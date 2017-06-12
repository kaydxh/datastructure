#include "bst.h"

#define Balanced(x) (stature((x).lc) == stature((x).rc)) //理想平衡
#define BalFac(x) (stature((x).lc) - stature((x).rc)) //平衡因子
#define AvlBalanced(x) ((-2 < BalFac(x)) && (BalFac(x) < 2)) //AVL平衡条件

//左右孩子中取更高者，如果高度相同，优先取与父亲同侧的孩子（同侧就是一条线上，可以用zig-zig或zag-zag）
#define tallerChild(x) ( \
	stature( (x)->lc) > stature( (x)->rc) ? (x)->lc : ( \ 
	stature( (x)->lc) < stature( (x)->rc) ? (x)->rc : ( \ 
	IsLChild(*(x)) ? (x)->lc : (x)->rc \ 
	) \
	) \
)


template <typename T>
class AVL : public BST<T> {
public:
	BinNode<T> * insert(const T& e);
	bool remove(const T& e);
};

template <typename T>
BinNode<T> * AVL<T>::insert(const T& e) {
	BinNode<T> * & x = this->search(e);
	if (x) { //如果要插入的目标已经存在就退出
		return x;
	}

	x = new(std::nothrow) BinNode<T>(e, this->_hot); //创建新节点
	if (NULL == x) {
		return NULL;
	}

	this->_size++;
	for (BinNode<T> * g = this->_hot; g; g = g->parent) {//从x的父亲向上出发，逐层检查各代祖先g
		if (!AvlBalanced(*g)) { //失衡
			printf("g[%d] hot[%d]\n", g->data, this->_hot->data);
            //这里等价于下面代码 
			//这里将g的父亲的孩子连接平衡后的局部子树，而平衡后的局部子树连接g的父亲在rotaeAt()函数内完成了。
			//平衡后，g的高度已经在rotaeAt内部的connect34更新
			if (!g->parent) {  //如果新的子树根节点没有父亲，就将_root赋值为平衡后的新的子树的根节点
    			this->_root = this->rotateAt ( tallerChild ( tallerChild ( g ) ) );  
    		} else if (IsLChild(*g))  {  //如果g是左孩子，通过g的父亲的左孩子连接平衡后的新的子树的根节点
    			BinNode<T>* p = g->parent;  
     			p->lc = this->rotateAt ( tallerChild ( tallerChild ( g ) ) );  
 			} else {  //如果g是右孩子，通过g的父亲的右孩子连接平衡后的新的子树的根节点
     			BinNode<T>* p = g->parent;  
     			p->rc = this->rotateAt ( tallerChild ( tallerChild ( g ) ) );  
 			}

			break;
		} else {
			this->updateHeight(g); //如果原来就是平衡的，高度也可能增加，比如，单节点插入一个界定啊后，高度有0变成了1。
							// 平衡因子毕竟有范围(-2<fac<2)，不是绝对一样，所以可能增加
		}
	} //最多一次调整，如果真做过调整，则全树高度必然复原

	return x;
}

template <typename T>
bool AVL<T>::remove (const T& e) {
	BinNode<T> * & x = this->search(e);
	if (!x) {
		return false;
	}

	removeAt(x, this->_hot); //按照BST规则删除节点x，此后，原节点之父_hot及其祖先均可能失衡
	this->_size--; //更新规模

	for (BinNode<T> * g = this->_hot; g; g = g->parent) {
		if (!AvlBalanced(*g)) { //失衡
			 //将g的父亲的孩子连接平衡后的局部子树
			if (!g->parent) {
				g = this->_root = this->rotateAt(tallerChild(tallerChild(g)));
			} else if (IsLChild(*g))  {  //如果g是左孩子，通过g的父亲的左孩子连接平衡后的新的子树的根节点
    			BinNode<T>* p = g->parent;  
     			g = p->lc = this->rotateAt ( tallerChild ( tallerChild ( g ) ) );  
 			} else {  //如果g是右孩子，通过g的父亲的右孩子连接平衡后的新的子树的根节点
     			BinNode<T>* p = g->parent;  
     			g = p->rc = this->rotateAt ( tallerChild ( tallerChild ( g ) ) );  
 			}
		}

		this->updateHeight(g);
	} //可能需要logn次调整，无论是否做过调整，全树高度均可能降低

	return true;
}

//若同样按照中序遍历的次数，平衡后的g(x)、p和v，将其命名为a、b和c，这一局部的中序遍历为{T0, a, T1, b, T2, c, T3}
// 局部子树与上层节点的双向连接均有上层调用者完成
template <typename T>
BinNode<T> * BST<T>::connect34(BinNode<T> * a, BinNode <T> * b, BinNode<T> * c,
	BinNode<T> * T0, BinNode<T> * T1, BinNode<T> * T2, BinNode<T> * T3) {
	a->lc = T0; //a的左孩子（左子树）设为T0
	if (T0) {
		T0->parent = a; //子树T0的父亲设置为节点a
	}

	a->rc = T1;
	if (T1) {
		T1->parent = a;
	}
	this->updateHeight(a); //左右孩子设置好后，就可以更新a的高了

	c->lc = T2;
	if (T2) {
		T2->parent = c;
	}

	c->rc = T3;
	if (T3) {
		T3->parent = c;
	}
	this->updateHeight(c);

	b->lc = a;
	a->parent = b;

	b->rc = c;
	c->parent = b;
	this->updateHeight(b); //左右孩子设置好后，就可以更新b的高了

	return b; //该子树新的根节点
}

//旋转后，新的最高节点的父亲会指向老的最高节点的父亲，但是反向的连接要由上层函数完成
//节点v有2个子树，p一个孩子是v，还有一个子树（左子树还是右子树都有可能），g一个孩子是p，还有一个子树（左子树还是右子树都有可能）
template <typename T>
BinNode<T> * BST<T>::rotateAt(BinNode<T> * v) {
	BinNode<T> * p = v->parent; //p是v的父亲
	BinNode<T> * g = p->parent; //g是p的父亲

	if (IsLChild(*p)) {//左孩子就zig
		if (IsLChild(*v)) { //也是左孩子，就是zig-zig, 这个旋转后，3个节点的顺序为v,p,g
			p->parent = g->parent; //向上连接。因为这里旋转后，p成了最高的节点。新的最高点的父亲连接老的最高点的父亲
			return connect34(v, p, g, v->lc, v->rc, p->rc, g->rc);
		} else { //zig-zag, 旋转后，3个节点的顺序为p,v,g
			v->parent = g->parent; //向上连接。因为zig-zag旋转后，v成了最高的节点。
			return connect34(p, v, g, p->lc, v->lc, v->rc, g->rc);
		}

	} else { //zag
		if (IsRChild(*v)) { //zag-zag
			p->parent = g->parent; 
			return connect34(g, p, v, g->lc, p->lc, v->lc, v->rc);
		} else { //zag-zig
			v->parent = g->parent; //新的最高节点v的父亲连接老的最高节点g的父亲
			return connect34(g, v, p, g->lc, v->lc, v->rc, p->rc);
		}

	}
}

//test
template <typename T>
void testBinTree () {
	AVL<T> avl;
	BinNode<T> * r = avl.insert(36);
    BinNode<T> * n1 = avl.insert(27);
    BinNode<T> * n2 = avl.insert(6);
    BinNode<T> * n3 = avl.insert(1);
	BinNode<T> * n4 = avl.insert(58);
	BinNode<T> * n5 = avl.insert(53);
	BinNode<T> * n6 = avl.insert(64);
	BinNode<T> * n7 = avl.insert(40);
	BinNode<T> * n8 = avl.insert(46);


	avl.travIn(VisitPrint<T>());
	printf("\n");

	avl.travPost(VisitPrint<T>());
	printf("\n");

	avl.travPre(VisitPrint<T>());
	printf("\n");

	avl.remove(27);
	avl.travIn(VisitPrint<T>());
	printf("\n");
}


int main() {

	BinNode<int> c;
	VisitPrint<int> v;

	testBinTree<int>();

	return 0;
}