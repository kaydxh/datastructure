#include <iostream>
using namespace std;

void swap(int &a, int &b) {
	int tmp = a;
	a = b;
	b = tmp;
}

void precolateDown(int *a, int len, int parent) {
	int child = 2*parent + 1;
	while (child < len) {
		//左孩子小于右孩子，就child++，得到右孩子索引
		if ((child + 1) < len && a[child] < a[child + 1]) {
			child++;
		}

		//a[child]为左右孩子中最大者
		if (a[child] > a[parent]) {//如果孩子的最大者大于父亲，那么就交换孩子和父亲
			::swap(a[child], a[parent]);
			parent = child; // 更新父亲索引，
			child = 2 * parent + 1; //更新左孩子索引
		} else { //如果父亲大于孩子的节点，那么堆秩序性就满足，然后break
			break;
		}
	}
}

//最大堆
void buildHeap(int* a, int n) {
	for (int i = n - 2; i >= 0; --i) {//从n-2开始，父节点最大为n-2（n的节点，最大节点为n-1，最大父节点为n-2）
		precolateDown(a, n, i);
	}
}

void heapSort(int* a, int n) {
	buildHeap(a, n);//最大堆建完后，最大者在第0位，所以需要和将第0位与最后一位交换（最大者归位），然后在进行下滤，保持堆秩序性
	int end = n - 1;
	while (end > 0) 
		::swap(a[0], a[end]);
		precolateDown(a, end, 0);
		--end;
	}
}


int main() {
	int a[] = {2,1,6,3,9,7,4,8,5};
	heapSort(a, sizeof(a)/sizeof(int));

	for (int i = 0; i < sizeof(a)/sizeof(int); ++i) {
		cout << a[i] << " ";
	}

	cout << endl;
}
