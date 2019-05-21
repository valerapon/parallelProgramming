#include <iostream>
#include <climits>
#include <unistd.h>
#include <sched.h>

void bubble(int *array, int n, int fdWrite) {
	for (int i = 0; i < n; ++i) {
		for (int j = 0; j < n - i - 1; ++j) {
			if (array[j] < array[j + 1]) {
				int tmp = array[j + 1];
				array[j + 1] = array[j];
				array[j] = tmp;
			}
		}
		write(fdWrite, array + n - i - 1, sizeof(int));
	}
} 

void startProcesses(int array[], int size, int **fd, int CORE) {
	for (int i = 0; i < CORE; ++i) {
		if (fork() == 0) {
			for (int j = 0; j < CORE; ++j) {
				if (i != j) {
					close(fd[j][0]);
					close(fd[j][1]);
				}
			}
			close(fd[i][0]);
			cpu_set_t mask;
			CPU_ZERO( &mask );
			CPU_SET(i, &mask);
			if (i != CORE - 1) {
				buble(array + i * (size / CORE), size / CORE, fd[i][1]);
			}
			else {
				buble(array + i * (size / CORE), size - i * (size / CORE), fd[i][1]);
			}
			close(fd[i][1]);
			exit(0);
		}
	}
}

void arrayFilling(int array[], int size, int **fd, int CORE) {
	int iter = 0, minEl, minId;
	int *countNumbers = new int[CORE];
	int *getter = new int[CORE];
	bool *used = new bool[CORE];
	
	for (int i = 0; i < CORE; ++i) {
		used[i] = true;
		getter[i] = INT_MAX;
		if (i != CORE - 1) {
			countNumbers[i] = size / CORE;
		}
		else {
			countNumbers[i] = size - i * (size / CORE);
		}
	}
	while (iter < size) {
		for (int i = 0; i < CORE; ++i) {
			if (used[i] && countNumbers[i] > 0) {
				read(fd[i][0], getter + i, sizeof(int));
				used[i] = false;
				--countNumbers[i];
			}
		}
		minEl = getter[0];
		minId = 0;
		for (int i = 1; i < CORE; ++i) {
			if (minEl > getter[i]) {
				minEl = getter[i];
				minId = i;
			}
		}
		array[iter] = minEl;
		getter[minId] = INT_MAX;
		++iter;
		used[minId] = true;
	}
	delete [] used;
	delete [] getter;
	delete [] countNumbers;
}

int main() {
	int CORE, *array, **fd, *getter, size, *countNumbers, iter, tmp, minEl, minId;
	bool *used;
	CORE = (int)sysconf(_SC_NPROCESSORS_ONLN);
	fd = new int*[CORE];
	for (int i = 0; i < CORE; ++i) {
		fd[i] = new int[2];
		pipe(fd[i]);
	}
	std::cin >> size;
	array = new int[size];
	for (int i = 0; i < size; ++i) {
		std::cin >> array[i];
	}
	startProcesses(array, size, fd, CORE);
	arrayFilling(array, size, fd, CORE);
	for (int i = 0; i < size; ++i) {
		std::cout << array[i] << ' ';
	}
	for (int i = 0; i < CORE; ++i) {
		close(fd[i][0]);
		close(fd[i][1]);
	}
	for (int i = 0; i < CORE; ++i) {
		delete [] fd[i];
	}
	delete [] fd;
	delete [] array;
	return 0;
}
