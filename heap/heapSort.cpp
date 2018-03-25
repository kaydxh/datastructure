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
		if (a[child] > a[parent]) {
			::swap(a[child], a[parent]);
			parent = child;
			child = 2 * parent + 1;
		} else {
			break;
		}
	}
}

//最大堆
void buildHeap(int* a, int n) {
	for (int i = n - 2; i >= 0; --i) {
		precolateDown(a, n, i);
	}
}

void heapSort(int* a, int n) {
	buildHeap(a, n);
	int end = n - 1;
	while (end > 0) {
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
