#include <iostream>
#include <algorithm>
#include <vector>
using namespace std;

template <typename T>
static bool lt(T *a, T *b) {
	return lt(*a, *b);
}

template <typename T>
static bool lt(T &a, T &b) {
	return a < b;
}

template <typename T>
struct PQ { //优先级队列PQ模板类
   virtual void insert ( T ) = 0; //按照比较器确定的优先级次序插入词条
   virtual T getMax() = 0; //取出优先级最高的词条
   virtual T delMax() = 0; //删除优先级最高的词条
};


//判断索引i是否有效，即PQ[i]是否合法
#define InHeap(n, i) ( ((-1) < (i)) && ((i) < (n)) ) 

//索引i的父节点索引
#define Parent(i) ((i - 1) >> 1) 

// 末节点的父亲（最后一个内部节点）
#define LastInternal(n) Parent(n - 1) 

// 左孩子的索引 2i(v) + 1
#define LChild(i) ( 1 + ((i) << 1) ) 

//右孩子 2i(v) + 2
#define RChild(i) ( (1 + (i)) << 1 ) 

// 判断PQ[i]是否有父亲，如果有父亲 i > 0
#define ParentVaild(i) (0 < i) 

//判断PQ[i]是否存在左孩子
#define LChildVaild(n, i) InHeap(n, LChild(i)) 

//判断PQ[i]是否存在右孩子
#define RChildVaild(n, i) InHeap(n, RChild(i)) 

//取大者的索引
#define Bigger(PQ, i, j)  (lt(PQ[i] , PQ[j]) ? j : i) 

//取父子（最多3个）中的最大者,  // 相等时取父节点的索引，可以避免不必要的交换
#define ProperParent(PQ, n, i) \
		( RChildVaild(n, i) ? Bigger(PQ, Bigger(PQ, i, LChild(i) ), RChild(i) ) : \
			(LChildVaild(n, i) ? Bigger(PQ, i, LChild(i) ) : i \
			) \
		)


template <typename T>
class PQ_ComplHeap : public PQ<T> {
protected:
	int percolateDown(int n, int i); //下滤
	int percolateUp(int i); //上滤
	void heapify(int n); //Floyd建堆算法

public:
	PQ_ComplHeap() {}
	PQ_ComplHeap(T * A, int n) {
		for (int i = 0; i < n; ++i) {
			elems.push_back(A[i]);
		}

		heapify(n);
	}

	void insert(T);
	T getMax();
	T delMax();

	void Print();

private:
	vector<T> elems;

};

template <typename T>
T PQ_ComplHeap<T>::getMax() {
	return elems[0]; //在最大堆的最大值（最高优先级）位于堆顶，即索引为0
}

template <typename T>
void PQ_ComplHeap<T>::insert(T e) {
	elems.push_back(e); //将新词条放入向量末尾
	percolateUp(elems.size() - 1); //再对该词条实施上滤调整
}

template <typename T>
int PQ_ComplHeap<T>::percolateUp(int i) {
	while (ParentVaild(i)) { //一旦i的父亲不存在就退出
		int j = Parent(i); //j为i的父亲的索引

		if (lt(elems[i], elems[j])) { //如果孩子i的值小于父亲j的值，就表示堆序正常，可以退出
			break;
		}

		swap(elems[i], elems[j]); //否则交换父子的值，保证父亲的值大于孩子的值
		i = j; //更新i的值为j，继续考察上一层的堆序
	}

	return i; //返回最终上滤抵达的位置
}

template <typename T>
T PQ_ComplHeap<T>::delMax() { 
	if (elems.size() <= 0) { 
		return -1;
	}

	T maxElem = elems[0]; //先备份最大值
	elems[0] = elems[elems.size() - 1]; //将最末尾的词条取代堆顶的最大值
	elems.pop_back();//将最后一个元素删除
	percolateDown(elems.size(), 0); //进行下滤，保证堆序性

	return maxElem; //返回删除的最大词条
}

template <typename T>
int PQ_ComplHeap<T>::percolateDown(int n, int i) {
	int j;
	while (i != (j = ProperParent(elems, n, i)) ) { //i不是父子中最大那个值的索引，就需要交换，让父亲始终保持最大
		swap(elems[i], elems[j]);
		i = j; //更新i为j，继续考察下降一层后的堆序
	}

	return i; //返回下滤抵达的位置
}

template <typename T>
void PQ_ComplHeap<T>::heapify(int n) {
	for (int i = LastInternal(n); InHeap(n, i); i--) {
		percolateDown(n, i);
	}
}

template <typename T>
void PQ_ComplHeap<T>::Print() {
	for (int i = 0; i < elems.size(); ++i) {
		cout << elems[i] << " ";
	}

	cout << endl;
}

int main() {

	int a[] = {2,1,6,3,9,7,4,8,5};
	PQ_ComplHeap<int> heap(a, sizeof(a)/sizeof(int));

	heap.Print(); //9 8 7 5 1 6 4 3 2 

	heap.insert(100);
	heap.Print(); //100 9 7 5 8 6 4 3 2 1 

	heap.delMax();
	heap.Print(); //9 8 7 5 1 6 4 3 2

	return 0;
}
