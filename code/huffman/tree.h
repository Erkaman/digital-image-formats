#ifndef _TREE_H_
#define _TREE_H_

#include <vector>
#include <map>

struct SymbolFreq{

public:
    unsigned long symbol;
    unsigned long freq;

    void print()const;
};

/* A node of the Huffman tree. */
struct Node {

private:
    SymbolFreq val;

    /* Values of nodes should not change, just move them around*/
    Node * left;
    Node * right;

    static const unsigned long emptySymbol = 0;

    bool NodePointerCompare(const Node *& a, const Node *& b);

    static void getCodeLengths(
        const Node * node,
	std::map<unsigned long, unsigned long> & codeLengths,
	int depth);

public:

    SymbolFreq getNodeValue()const;

    Node * getLeft()const;
    Node * getRight()const;

    Node();
    Node(SymbolFreq val_);
    Node(Node * leftNode, Node * rightNode);

    /* A single node has a depth of 0 */
    int getMaxDepth()const;

    bool isSingle()const;

    void print()const;

    unsigned long getMaxSymbol()const;

    std::vector<unsigned long> getCodeLengths(unsigned long alphabetSize)const;

    ~Node();
};

#endif /* _TREE_H_ */
