#ifndef INTENT_CLASSIFIER
#define INTENT_CLASSIFIER

#include "svm.h"
#include <string>
#include <vector>

using namespace std;

//Multiclass SVM
class IntentClassifier
{
private:
	svm_problem prob;
	svm_parameter* param;
	svm_model* model;

public:
	long numExamples;
	int numClasses;
	int numFeatures;
	string name;


	IntentClassifier();
	//For Creation with default params
	IntentClassifier(string dataFilename, int numClasses, int numFeatures);
	//Only for classifying examples with an existing model
	IntentClassifier(string modelFilename);
	~IntentClassifier();

	double predict(svm_node* example, bool predictProb);
	void loadDataSet(string filename, int numClasses, int numFeatures);

	void createModel();
	void loadModel(string filename);
	void writeModelToFile(string filename, svm_model* m);

	/////////////////////////////////////////////////////
	//Accessors
	svm_problem getProblem(){return this->prob;};
	svm_parameter* getParam(){return this->param;};
	/////////////////////////////////////////////////////

	/////////////////////////////////////////////////////
	//Mutators
	/*
	*TYPE
	*0 -- linear (u'*v)
	*1 -- polynomial: (gamma*u'*v + coef0)^degree
	*2 -- radial basis function: exp(-gamma*|u-v|^2)
	*3 -- sigmoid: tanh(gamma*u'*v + coef0)
	*4 -- precomputed kernel (kernel values in training_set_file)
	*/
	void setKernelFunction(int type, double degree, double gamma, double coef0);
	//IMPORTANT: cost value is the penalty for forming separating hyperplanes that
	//missclassify data points.  The > the cost value, the more accurate the model,
	//but it can take a hell of a long time to compute with many classes/examples and will
	//overfit the data
	void setCostValue(double cost);
	//Error must be no greater than this val before terminating the computation
	void setStoppingTolerance(double epsilon);
	/////////////////////////////////////////////////////
};

#endif