/*
*Class for putting data into a format that can be passed to an IntentClassifier(SVM)
*
*PASS ALL VECTORS of data in exact order of feauture labels.  
*
*Example: classifying XYZ
*coordinates.  X = feature #1 (label = 1), Y = feature #2 (label = 2), Z = feature #3
*(label = 3).  The vector must be ordered like this vector(X,Y,Z).  This assigns label 1
* to X, label 2 to Y, etc...
*
*You can also specify a list of vectors; this follows the same rule as above.  The n values
*of the first vector will have labels 1 to n, the k values of vector 2 will have lables
*n+1 to n+k, etc...
*
*/

#ifndef SVM_FORMATTER
#define SVM_FORMATTER

#include "svm.h"
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>

using namespace std;

//SVM NODE
/*
*int index (feature number 0 to 1)
*double value (normalized for the indexed feature)
*/

//EXAMPLE
/*
*Sparse array of features with value != 0
*and terminated by svm_node with index = -1
*/

string svmNodeToString(svm_node s)
{
	stringstream tmp;
	tmp << s.index << ':' << s.value;
	return tmp.str();
};

struct Example
{
	svm_node* example;
	int classification;
	int numnodes;

	string toString()
	{
		stringstream tmp;
		tmp << classification;
		for(int i = 0; i < this->numnodes; i++)
			tmp << " " << svmNodeToString(example[i]) << " ";

		return tmp.str();
	};
};

Example vectorToExample(int classification, vector<double> featuresVals)
{
	vector<int> indicies;
	for(int i = 0; i < featuresVals.size(); i++)
	{
		if(featuresVals.at(i) != 0)
			indicies.push_back(i);
	}
	svm_node* ex = (svm_node*)malloc(sizeof(svm_node)*(indicies.size()+1));;

	for(int i = 0; i < indicies.size(); i++)
	{
		svm_node node = svm_node();
		node.index = indicies.at(i)+1;
		node.value = featuresVals.at(i);
		ex[i] = node;
	}
	svm_node delim = svm_node();
	delim.index = -1;
	ex[indicies.size()] = delim;

	Example e = Example();
	e.classification = classification;
	e.example = ex;
	e.numnodes = indicies.size();
	return e;
};

Example listToExample(int classification, vector<vector<double>> featuresVals)
{
	vector<double> example = vector<double>();
	for(int i = 0; i < featuresVals.size(); i++)
	{
		vector<double> v = featuresVals.at(i);
		example.insert(example.end(),v.begin(),v.end());
	}
	return vectorToExample(classification,example);
};

void examplesToFile(string filename, vector<Example> examples)
{
	ofstream out;

	out.open(filename);
	if(out.is_open())
	{
		for(int i = 0; i < examples.size(); i++)
			out << examples.at(i).toString() << endl;
		out.close();
	}
};

class RealTimeClassifier
{
public:
	vector<Example> examples;
	bool classifying;
	bool finished;
	int currLabel;
	int numExamples, numClasses;
	int numExMax, numClassMax;

	RealTimeClassifier()
	{
		this->examples = vector<Example>();
		this->classifying = this->finished = false;
		this->currLabel = 0;
		this->numExamples = 0;
		this->numClasses = 1;
		this->numExMax = 100;
	};

	RealTimeClassifier(int numClasses, int maxExamples)
	{
		this->examples = vector<Example>();
		this->classifying = this->finished = false;
		this->currLabel = 0;
		this->numExamples = this->numClasses = 0;
		this->numClassMax = numClasses;
		this->numExMax = maxExamples;
	};


	void classify(vector<double> v)
	{
		if(!this->finished)
		{
			if(!this->classifying)
			{
				int label;
				cin >> label;
				this->currLabel = label;
				this->classifying = true;
			}
			this->examples.push_back(vectorToExample(this->currLabel,v));
			this->numExamples++;
		}
	};

	void classify(vector<vector<double>> v)
	{
		if(!this->finished)
		{
			if(!this->classifying)
			{
				int label;
				cin >> label;
				this->currLabel = label;
				this->classifying = true;
			}
			this->examples.push_back(listToExample(this->currLabel,v));
			this->numExamples++;
		}
	};

	void stopClassifying()
	{
		this->classifying = false;
		this->numExamples = 0;
		this->numClasses++;
		if(this->numClasses >= this->numClassMax)
		{
			this->writeToFile("test.txt");
			this->finished = true;
		}
	};

	bool done()
	{
		if(this->numExamples >= this->numExMax)
			return true;
		return false;
	};

	void writeToFile(string filename)
	{
		examplesToFile(filename,this->examples);
	};
};

#endif