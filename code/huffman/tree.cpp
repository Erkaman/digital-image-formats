#include "tree.h"
#include <cstddef>
#include "../io.h"
#include <cctype>
#include <algorithm>

using std::vector;
using std::sort;
using std::map;
using std::max;

bool nodePointerCompare(const Node * a, const Node * b);

bool nodePointerCompare(const Node * a, const Node * b)
{
    return (a->getNodeValue().symbol < b->getNodeValue().symbol);
}

void SymbolFreq::print()const
{
    if(!isprint(this->symbol))
        verbosePrint("non-printable:%ld:%ld\n",
                     this->symbol,
                     this->freq);
    else
        verbosePrint("non-printable:%c:%ld\n",
                     this->symbol,
                     this->freq);
}

bool Node::isSingle()const
{
    return (this->left == NULL && this->right == NULL);
}


const unsigned long Node::emptySymbol;

void Node::print()const
{
    if(this->left != NULL)
        this->left->print();

    this->val.print();

    if(this->right != NULL)
        this->right->print();
}

Node::Node(): left(NULL), right(NULL)
{
    val.symbol = emptySymbol;
    val.freq = emptySymbol;
}


Node::Node(SymbolFreq val_): left(NULL), right(NULL)
{
    this->val = val_;
}

Node::~Node()
{
    if(this->left != NULL)
        delete this->left;

    if(this->right != NULL)
        delete this->right;
}

Node::Node(Node * leftNode, Node * rightNode)
{
    /* A deep copy isn't needed, since the nodes are just moved
       around during the algorithm. Since they are then into one big,
       this big can be dealt with at the end of the algorithm*/
    this->left = leftNode;
    this->right = rightNode;

    this->val.freq = leftNode->val.freq + rightNode->val.freq;
    this->val.symbol = emptySymbol;
}

SymbolFreq Node::getNodeValue()const
{
    return val;
}

Node * Node::getLeft()const
{
    return this->left;
}

Node * Node::getRight()const
{
    return this->right;
}

int Node::getMaxDepth()const
{
    int lDepth;
    int rDepth;

    if (this->isSingle())
        return 1;
    else {
        lDepth = this->left->getMaxDepth();
        rDepth = this->right->getMaxDepth();

        if (lDepth > rDepth)
            return(lDepth+1);
        else
            return(rDepth+1);
    }
}


void Node::getCodeLengths(
    const Node * node,
    /* A symbol value to a code length.*/
    map<unsigned long, unsigned long> & codeLengths,
    int depth)
{
    if(node->isSingle()){
	codeLengths[node->val.symbol] = depth;
/*	codeLengths.push_back(depth); */
        return;
    }

    if(node->getRight() != NULL)
        getCodeLengths(
            node->getRight(),
	    codeLengths,
            depth+1);

    if(node->getLeft() != NULL)
        getCodeLengths(
            node->getLeft(),
	    codeLengths,
            depth+1);

}

std::vector<unsigned long> Node::getCodeLengths(unsigned long alphabetSize)const
{
    map<unsigned long, unsigned long> m;

    getCodeLengths(this, m,0);

    vector<unsigned long> codeLengths;

    for(size_t i = 0; i < alphabetSize; ++i){
	if(m.find(i) != m.end())
	    codeLengths.push_back(m[i]);
	else
	    codeLengths.push_back(0);
    }

    return codeLengths;
}

unsigned long Node::getMaxSymbol()const
{
    if(this->isSingle()){
	return this->val.symbol;
    } else {
	if(left == NULL && right != NULL){
	    return right->getMaxSymbol();
	} else if(right == NULL && left != NULL){
	    return left->getMaxSymbol();
	} else {
	    return max(left->getMaxSymbol(), right->getMaxSymbol());
	}
    }
}
