#include "bst.h"

#define IsBlack(p) (!(p) || (RB_BLACK == (p)->color)) //若是根节点，这就是黑色，否则判断颜色是否是黑色
#define IsRed(p) (!IsBlack(p)) //非黑即红

//节点x的黑高度更新条件，x的左右孩子的黑高度相等，且x若是红色，则x的左（右）孩子的黑高度就是x的高度，否则x是黑色，
//那么x的黑高度就是孩子的高度 + 1
//注意：若BlackHeightUpdate为false，就是x的黑高度不平衡，否则，x的黑高度平衡
#define BlackHeightUpdate(x) ( \
	(stature((x).lc)  == stature((x).rc) ) && \
	((x).height == (IsRed(&x) ? stature((x).lc) : stature((x).lc) + 1)) \
)

#if 0
template<typename T>
bool BlackHeightUpdate(BinNode<T> x) {
	return (stature((x).lc)  == stature((x).rc) ) && 
	((x).height == (IsRed(&x) ? stature((x).lc) : stature((x).lc) + 1));
}
#endif

template <typename T>
class RedBlack : public BST<T> {
protected:
	void solveDoubleRed(BinNode<T> * x);
	void solveDoubleBlack(BinNode<T> * x);
	int updateHeight(BinNode<T> * x);

public:
	BinNode<T> * insert(const T& e);
	bool remove(const T& e);
};

//更新节点高度，注意引统一定义的stature(NULL) = -1,所以height比黑高度少1，但是不影响各种算法的比较判断
template <typename T>
int RedBlack<T>::updateHeight(BinNode<T> * x) {
	x->height = max(stature(x->lc), stature(x->rc));//孩子一般黑高度相等，除非出现双黑
	return IsBlack(x) ? x->height++ : x->height; //若当前节点为黑，则计入黑高度
}

template <typename T>
BinNode<T> * RedBlack<T>::insert(const T& e) {
	BinNode<T> * & x = this->search(e); //关键码e如果存在就直接退出（注意这里返回的引用，因为新节点将在x位置接入，
		//且x与它父亲是已经连接的）
	if (x) {
		return x; //注意无论e是否存在原树中，返回时总有x->data == e
	}

	x = new(std::nothrow) BinNode<T>(e, this->_hot, NULL, NULL, -1);//创建红色节点x，以_hot为父亲，黑高度为-1
	if (NULL == x) {
		return NULL;
	}

	this->_size++;
	solveDoubleRed(x); //双红修正
	return x ? x : this->_hot->parent;
}

template <typename T>
void RedBlack<T>::solveDoubleRed(BinNode<T> * x) { //x当前必为红色
	if (IsRoot(*x)) {//若已转至（或者递归到）树根，则将其变为黑色，整树的高度也+1
		this->_root->color = RB_BLACK;
		this->_root->height++;
		return;
	}

	BinNode<T> * p = x->parent;
	if (IsBlack(p)) { //p为黑色，即不存在双红，直接退出
		return;
	}

	BinNode<T> * g = p->parent;
	BinNode<T> * u = uncle(x);
	if (IsBlack(u)) { //如果x的叔父是黑色
		if (IsLChild(*x) == IsLChild(*p)) { //若x与p同侧（即zig-zig或者zag-zag），注意这里用是否都是左孩子判断
				//ture == ture 的情况就zig-zig， false == false 的情况即zag-zag
			p->color = RB_BLACK; //p转至黑色，x保持红色
		} else { //否则，x与p异侧（即zig-zag或者zag-zig）
			x->color = RB_BLACK;
		}

		g->color = RB_RED;// g必定有黑转红

		BinNode<T> * gg = g->parent;

		BinNode<T> * r = NULL;
		if (!g->parent) {
			r = this->_root = this->rotateAt(x);
		} else if (IsLChild(*g))  { 
			r = g->parent->lc = this->rotateAt(x);
		} else {
			r = g->parent->rc = this->rotateAt(x);
		}

		r->parent = gg; //调整之后的子树的新根节点与原曾祖父（g的父亲）连接
	} else { //u为红色
		p->color = RB_BLACK; // p由红转黑
		p->height++;

		u->color = RB_BLACK; //u由红转黑
		u->height++;

		if ( !IsRoot(*g)) { //g不是根节点的话，就转为红色，否则，在下次递归中，转为黑色，退出
			g->color = RB_RED;
		}

		solveDoubleRed(g); //继续递归(类似于尾递归)，若
	}
}

template <typename T>
bool RedBlack<T>::remove(const T& e) {
	BinNode<T> * & x = this->search(e);
	if (!x) {
		return false;
	}

	BinNode<T> * r = removeAt(x, this->_hot);//删除x返回x的直接后继
	if (!(--this->_size)) {//若删除的节点x是最后一个节点（当然也是根节点，删除后为空树），就直接返回
		return true;
	}

	if (!this->_hot) {// 若被删除的是根节点，（根节点的父亲_hot为NULL），此时需要将新的根节点（_root）染成黑色
					//并更新黑高度
		this->_root->color = RB_BLACK;
		this->updateHeight(this->_root);

		return true;
	}

	 //若删除的不是根节点

	if (BlackHeightUpdate(*this->_hot)) {//若_hot的黑高度依然平衡（_hot黑高度平衡，意味着所有祖先的黑高度也平衡）
			//此时无需调整，直接返回。注意这种情况意味着x为红色（r必为黑色，因为条件3）
		return true;
	}

	if (IsRed(r)) { //若r为红色(x必为黑色，条件3)，只需将r转至为黑色，并更新黑高度，直接退出，这里黑高度保持不变，因为x此时为黑色，
					//删除后有r（转至为黑色）补上。
		r->color = RB_BLACK;
		r->height++;

		return true;
	}

	// x和r均为黑色（双黑）
	solveDoubleBlack(r);

	return true;
}

template <typename T>
void RedBlack<T>::solveDoubleBlack(BinNode<T> * r) {
	BinNode<T> * p = r ? r->parent : this->_hot; // 删除x后，r的父亲变为p，如果p不存在，就返回
	if (!p) {
		return;
	}

	BinNode<T> * s = (r == p->lc) ? p->rc : p->lc; //s为r的兄弟
	if (IsBlack(s)) { //s为黑色
		BinNode<T> * t = NULL; //t为s的红孩子（如果左右孩子都红色，左孩子优先），如图没有红孩子仍然为NULL
		if (IsRed(s->rc)) {
			t = s->rc;
		}

		if (IsRed(s->lc)) {
			t = s->lc;
		}

		if (t) { //黑s有红孩子 BB-1 ，对节点t、s和p实施“3+4”重构。s继承p此前的颜色，t和p染成黑色。
			RBColor oldColor = p->color; //备份p之前的颜色

			BinNode<T> * b = NULL; //b为新子树的根，旋转后新的子树根与原子树根的父亲互联
			if (!p->parent) { //p是树根
				b = this->_root = this->rotateAt(t);
			} else if (IsLChild(*p))  { //p是左孩子
				b = p->parent->lc = this->rotateAt(t); 
			} else { //p是右孩子
				b = p->parent->rc = this->rotateAt(t);
			}

			if (HasLChild(*b)) { //旋转后，如果b是左孩子，就将b的颜色染黑，更新子树黑高度
				b->lc->color = RB_BLACK;
				this->updateHeight(b->lc);
			}

			if (HasRChild(*b)) { //旋转后，如果b是有孩子，就将b的颜色染黑，更新子树黑高度
				b->rc->color = RB_BLACK;
				this->updateHeight(b->rc);
			}

		} else { // 黑s无红孩子
			s->color = RB_RED; // s转红，同时黑高度减1 （BB-2-R 和 BB-2-B共同点是节点s由黑转红）
			s->height--;

			if (IsRed(p)) { // BB-2-R p颜色染黑，这里黑高度不变，因为染成黑后，p和s是虚线，提升变化后和p是红色的黑高度一样的
				p->color = RB_BLACK;
			} else { // BB-2-B  节点s已经在前面由黑转红，这里p是黑色，而s是红色，所以p到s是虚线，提升变化后，黑高度是降1
				p->height--;
				solveDoubleBlack(p); //递归上溯
			}
		}

	} else { //兄弟s为红 BB-3 以节点p为轴旋转，并交换节点s与p的颜色
		s->color = RB_BLACK;
		p->color = RB_RED;

		BinNode<T> * t = IsLChild(*s) ? s->lc : s->rc; //取t与其父s同侧
		this->_hot = p; //s的父亲

		if (!p->parent) { //p是树根
			this->_root = this->rotateAt(t);
		} else if (IsLChild(*p))  { //p是左孩子
			p->parent->lc = this->rotateAt(t); 
		} else { //p是右孩子
			p->parent->rc = this->rotateAt(t);
		}

		solveDoubleBlack(r);
	}
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
	RedBlack<T> rb;
	BinNode<T> * r = rb.insert(36);
    BinNode<T> * n1 = rb.insert(27);
    BinNode<T> * n2 = rb.insert(6);
    BinNode<T> * n3 = rb.insert(1);
	BinNode<T> * n4 = rb.insert(58);
	BinNode<T> * n5 = rb.insert(53);
	BinNode<T> * n6 = rb.insert(64);
	BinNode<T> * n7 = rb.insert(40);
	BinNode<T> * n8 = rb.insert(46);


	rb.travIn(VisitPrint<T>());
	printf("\n");

	rb.travPost(VisitPrint<T>());
	printf("\n");

	rb.travPre(VisitPrint<T>());
	printf("\n");

	rb.remove(27);
	rb.travIn(VisitPrint<T>());
	printf("\n");
}


int main() {

	BinNode<int> c;
	VisitPrint<int> v;

	testBinTree<int>();

	return 0;
}