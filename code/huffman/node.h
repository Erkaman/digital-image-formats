/* A node of the Huffman tree. */

#ifndef _NODE_H_
#define _NODE_H_

typedef unsigned int Symbol;
typedef unsigned int Freq;

struct SymbolFreq{
public:
    Symbol symbol;
    Freq freq;
};

bool symbolFreqCmp(const SymbolFreq & a, const SymbolFreq & b);

struct Node {

public:
    SymbolFreq val;

    static const Symbol emptySymbol = 0;

    /* Values of nodes should not change, just move them around*/
    Node * left;
    Node * right;

    Node(SymbolFreq val_);

    Node(Node * left_, Node * right_);

    ~Node();    

    bool isSingle()const;

};

#endif /* _NODE_H_ */
