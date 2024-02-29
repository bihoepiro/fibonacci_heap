#include <iostream>
#include <vector>
#include <cmath>

using namespace std;

template <class T>
struct ListNode {
    T data;
    // para saber el grado del nodo
    int degree;
    // para saber si el nodo ha perdido un hijo o no.
    bool marked;
    ListNode<T>* parent;
    ListNode<T>* child;
    ListNode *prev;
    ListNode *next;

    ListNode(const T& d) : data(d), degree(0), marked(false), parent(nullptr), child(nullptr), prev(this), next(this) {}
};

template <class T>
class FibonacciHeap {
private:
    ListNode<T> *minimum; // saber el nodo mínimo
    int numNodes;
public:
    FibonacciHeap() : minimum(nullptr), numNodes(0) {}

    // MINIMO
    ListNode<T> *getMinimum() const {
        return minimum;
    }

    bool isEmpty() const {
        // si no hay min, no hay fh por min-heap
        return minimum == nullptr;
    }

    // INSERTAR
    void insert(int data) {
        ListNode<T> *node = new ListNode(data);
        if (minimum == nullptr) {
            minimum = node;
        } else {
            node->next = minimum;
            node->prev = minimum->prev;
            minimum->prev->next = node;
            minimum->prev = node;
            // si el nodo es menor al nodo mínimo se asigna el nuevo nodo como mínimo
            if (data < minimum->data) {
                minimum = node;
            }
        }
        numNodes++;
    }

    // ENLAZAR
    void link(ListNode<T> *y, ListNode<T> *x) {
        // enlazar un nodo 'y' como hijo de un nodo 'x' en FH
        y->prev->next = y->next;
        y->next->prev = y->prev;
        y->prev = y;
        y->next = y;
        y->parent = x;
        if (x->child == nullptr) {
            x->child = y;
        } else {
            y->next = x->child;
            y->prev = x->child->prev;
            x->child->prev->next = y;
            x->child->prev = y;
        }
        x->degree++;
        //nodo 'y' es un nuevo hijo de nodo 'x', entonces ya no se considera un hijo perdido de otro nodo.
        y->marked = false;
    }

    // CONSODILATE
    // para que no queden arboles con el mismo grado.
    void consolidate() {
        // calculo de grado máximo del montículo
        int mDegree = static_cast<int>(log2(numNodes)) + 1;
        // nodo raiz de cada arbol de fh
        vector<ListNode<T> *> degreeArr(mDegree, nullptr);

        ListNode<T> *current = minimum;
        do {
            ListNode<T> *x = current;
            current = current->next;

            int degree = x->degree;
            while (degreeArr[degree] != nullptr) {
                ListNode<T> *y = degreeArr[degree];
                if (x->data > y->data) {
                    ListNode<T> *temp = x;
                    x = y;
                    y = temp;
                }
                // nodo 'y' como hijo de nodo 'x' si tienen el mismo grado.
                link(y, x);
                degreeArr[degree] = nullptr;
                degree++;
            }
            degreeArr[degree] = x;
        } while (current != minimum);

        minimum = nullptr;

        for (ListNode<T> *y: degreeArr) {
            if (y != nullptr) {
                if (minimum == nullptr) {
                    minimum = y;
                } else {
                    y->prev->next = y->next;
                    y->next->prev = y->prev;
                    y->prev = minimum;
                    y->next = minimum->next;
                    minimum->next->prev = y;
                    minimum->next = y;
                    if (y->data < minimum->data) {
                        minimum = y;
                    }
                }
            }
        }
    }

    // EXTRACT-MIN
    ListNode<T> *extractMin() {
        ListNode<T> *z = minimum;
        if (z != nullptr) {
            // verifica si tiene hijos o no
            if (z->child != nullptr) {
                ListNode<T> *child = z->child;
                do {
                    ListNode<T> *nextChild = child->next;
                    child->prev->next = child->next;
                    child->next->prev = child->prev;
                    child->prev = minimum;
                    child->next = minimum->next;
                    minimum->next->prev = child;
                    minimum->next = child;
                    child->parent = nullptr;
                    child = nextChild;
                } while (child != z->child);
            }
            z->prev->next = z->next;
            z->next->prev = z->prev;
            if (z == z->next) {
                minimum = nullptr;
            } else {
                minimum = z->next;
                consolidate();
            }
            numNodes--;
        }
        return z;
    }

    // UNION
    void unionFH(FibonacciHeap<T> &fh2) {
        // caso 0: si está vacion fh2, no se hace nada
        if (fh2.isEmpty()) return;
        //caso 1: si fh1 está vacío, se añade fh2.
        if (isEmpty()) {
            minimum = fh2.minimum;
        } else {
            // caso 2: ninguno de los fh está vacío
            ListNode<T> *lfh1 = minimum->prev;
            ListNode<T> *lfh2 = fh2.minimum->prev;
            lfh1->next = fh2.minimum;
            fh2.minimum->prev = lfh1;
            minimum->prev = lfh2;
            lfh2->next = minimum;
            // cual minimo es menor para asignar el min del fh unido
            if (fh2.minimum->data < minimum->data) {minimum = fh2.minimum;}
        }
        numNodes += fh2.numNodes;
        fh2.minimum = nullptr;
        fh2.numNodes = 0;
    }

    // CUT
    // cortar nodo de su padre
    void cut(ListNode<T>* child, ListNode<T>* parent) {
        child->marked = false;
        if (child->next == child) {
            parent->child = nullptr;
        } else {
            child->next->prev = child->prev;
            child->prev->next = child->next;
            if (parent->child == child) {
                parent->child = child->next;
            }
        }
        parent->degree--;
        minimum->prev->next = child;
        child->prev = minimum->prev;
        minimum->prev = child;
        child->next = minimum;
        child->parent = nullptr;
    }

    // CASCADING CUT
    void cascadingCut(ListNode<T>* node) {
        ListNode<T>* parent = node->parent;
        if (parent != nullptr) {
            if (!node->marked) {
                node->marked = true;
            } else {
                cut(node, parent);
                cascadingCut(parent);
            }
        }
    }

    // DISMINUIR CLAVE
    void decreaseKey(ListNode<T>* node, T newData) {
        // caso 1: la nueva data es mayor
        if (newData > node->data) {
            //no se puede disminuir la clave
            return;
        }
        // caso 2: la data es menor
        node->data = newData;
        ListNode<T>* parent = node->parent;
        // caso 2.1: la data nueva es menor a la del padre
        if (parent != nullptr && node->data < parent->data) {
            // intercambian lugares
            cut(node, parent);
            cascadingCut(parent);
        }
        // verificar si la nueva data del nodo es menor al minimo
        if (node->data < minimum->data) {
            minimum = node;
        }
    }

    // DELETE 'X'
    void deleteNode(ListNode<T>* node) {
        decreaseKey(node, std::numeric_limits<T>::min());
        extractMin();
    }

};

int main() {
    // Crear un montículo de Fibonacci de enteros
    FibonacciHeap<int> fibHeap;

    // Insercción
    fibHeap.insert(5);
    fibHeap.insert(10);
    fibHeap.insert(3);
    fibHeap.insert(7);

    // Obtener el mínimo del montículo
    ListNode<int>* minNode = fibHeap.getMinimum();
    cout << "Mínimo del montículo: " << minNode->data << endl;


    // Eliminar el mínimo del montículo
    ListNode<int>* extractedMin = fibHeap.extractMin();
    cout << "Mínimo extraído del montículo: " << extractedMin->data << endl;

    // Después de extraer el mínimo, obtener el nuevo mínimo
    minNode = fibHeap.getMinimum();
    cout << "Nuevo mínimo del montículo: " << minNode->data << endl;


    // Montículo F 2
    FibonacciHeap<int> fibHeap2;

    // Insercción 2.0
    fibHeap2.insert(12);
    fibHeap2.insert(1);
    fibHeap2.insert(8);

    // Union
    fibHeap.unionFH(fibHeap2);

    // Obtener el mínimo del montículo unido
    ListNode<int>* minNode1 = fibHeap.getMinimum();
    cout << "Mínimo del montículo unido: " << minNode1->data << endl;

    // Eliminación un nodo específico
    ListNode<int>* nodeToDelete = nullptr;
    ListNode<int>* current = fibHeap.getMinimum();
    do {
        if (current->data == 7) {
            nodeToDelete = current;
            break;
        }
        current = current->next;
    } while (current != fibHeap.getMinimum());


    fibHeap.deleteNode(nodeToDelete);
    cout << "Nodo con valor 7 eliminado." << endl;


    minNode = fibHeap.getMinimum();
    cout << "Nuevo mínimo del montículo: " << minNode->data << endl;

}
