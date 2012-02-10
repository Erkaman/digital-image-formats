#include "node.h"
#include <cstdio>

Node::Node(SymbolFreq val_):val(val_), left(NULL), right(NULL){}

Node::Node(Node * left_, Node * right_):left(left_), right(right_)
{
    this->val.symbol = emptySymbol;
    this->val.freq =
        this->left->val.freq + this->right->val.freq;
}

bool Node::isSingle()const
{
    return (this->left == NULL && this->right == NULL);
}

Node::~Node()
{
    if(this->left != NULL)
	delete this->left;
    if(this->right != NULL)
	delete this->right;
}

bool symbolFreqCmp(const SymbolFreq & a, const SymbolFreq & b)
{
    return a.freq < b.freq;
}

