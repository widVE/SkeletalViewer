#include "stdafx.h"
#include "IntentClassifier.h"
#include <iostream>
#include <fstream>
#include <sstream>


//////////////////////////////////////////////////////////
//HELPER FUNCTIONS
vector<string> tokenize(string line, char delim)
{
	string token;
	vector<string> tokens;
	std::istringstream iss(line);
	while(getline(iss, token, delim))
	{
		tokens.push_back(token);
	}
	return tokens;
};

extern "C" 
{
	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
	#include <ctype.h>
	#include <errno.h>
	#include "svm.h"
	#define Malloc(type,n) (type *)malloc((n)*sizeof(type))

	void print_null(const char *s) {}

	void exit_input_error(int line_num)
	{
		fprintf(stderr,"Wrong input format at line %d\n", line_num);
		exit(1);
	}
	svm_problem read_problem(const char *filename);
	struct svm_node *x_space;
	int cross_validation;
	int nr_fold;

	static char *line = NULL;
	static int max_line_len;

	static char* readline(FILE *input)
	{
		int len;
	
		if(fgets(line,max_line_len,input) == NULL)
			return NULL;

		while(strrchr(line,'\n') == NULL)
		{
			max_line_len *= 2;
			line = (char *) realloc(line,max_line_len);
			len = (int) strlen(line);
			if(fgets(line+len,max_line_len-len,input) == NULL)
				break;
		}
		return line;
	}

	// read in a problem (in svmlight format)

	svm_problem read_problem(const char *filename)
	{
		struct svm_problem prob;
		int elements, max_index, inst_max_index, i, j;
		FILE *fp = fopen(filename,"r");
		char *endptr;
		char *idx, *val, *label;

		if(fp == NULL)
		{
			fprintf(stderr,"can't open input file %s\n",filename);
			exit(1);
		}

		prob.l = 0;
		elements = 0;

		max_line_len = 1024;
		line = Malloc(char,max_line_len);
		while(readline(fp)!=NULL)
		{
			char *p = strtok(line," \t"); // label

			// features
			while(1)
			{
				p = strtok(NULL," \t");
				if(p == NULL || *p == '\n') // check '\n' as ' ' may be after the last feature
					break;
				++elements;
			}
			++elements;
			++prob.l;
		}
		rewind(fp);

		prob.y = Malloc(double,prob.l);
		prob.x = Malloc(struct svm_node *,prob.l);
		x_space = Malloc(struct svm_node,elements);

		max_index = 0;
		j=0;
		for(i=0;i<prob.l;i++)
		{
			inst_max_index = -1; // strtol gives 0 if wrong format, and precomputed kernel has <index> start from 0
			readline(fp);
			prob.x[i] = &x_space[j];
			label = strtok(line," \t\n");
			if(label == NULL) // empty line
				exit_input_error(i+1);

			prob.y[i] = strtod(label,&endptr);
			if(endptr == label || *endptr != '\0')
				exit_input_error(i+1);

			while(1)
			{
				idx = strtok(NULL,":");
				val = strtok(NULL," \t");

				if(val == NULL)
					break;

				errno = 0;
				x_space[j].index = (int) strtol(idx,&endptr,10);
				if(endptr == idx || errno != 0 || *endptr != '\0' || x_space[j].index <= inst_max_index)
					exit_input_error(i+1);
				else
					inst_max_index = x_space[j].index;

				errno = 0;
				x_space[j].value = strtod(val,&endptr);
				if(endptr == val || errno != 0 || (*endptr != '\0' && !isspace(*endptr)))
					exit_input_error(i+1);

				++j;
			}

			if(inst_max_index > max_index)
				max_index = inst_max_index;
			x_space[j++].index = -1;
		}

		fclose(fp);
		return prob;
	}

}
//////////////////////////////////////////////////////////

IntentClassifier::IntentClassifier()
{
	this->prob = svm_problem();
	this->param = new svm_parameter();
	//BEST PARAMETER DEFAULTS FOR OPTIMIZATION PROBLEM
	this->param->svm_type = C_SVC;
	this->param->kernel_type = RBF;
	this->param->degree = 3;
	this->param->coef0 = 0;
	this->param->nu = 0.5;
	this->param->cache_size = 100;
	this->param->C = 1;
	this->param->eps = 0.001;
	this->param->p = 0.1;
	this->param->shrinking = 1;
	this->param->probability = 0;
	this->param->nr_weight = 0;
	this->param->weight_label = NULL;
	this->param->weight = NULL;
	this->model = (svm_model*)malloc(sizeof(svm_model*));
}

IntentClassifier::IntentClassifier(string dataFilename, int numClasses, int numFeatures)
{
	this->prob = svm_problem();
	this->param = new svm_parameter();
	//BEST PARAMETER DEFAULTS FOR OPTIMIZATION PROBLEM
	this->param->svm_type = C_SVC;
	this->param->kernel_type = RBF;
	this->param->degree = 3;
	this->param->coef0 = 0;
	this->param->nu = 0.5;
	this->param->cache_size = 100;
	this->param->C = 1000;
	this->param->eps = 0.001;
	this->param->p = 0.1;
	this->param->shrinking = 1;
	this->param->probability = 0;
	this->param->nr_weight = 0;
	this->param->weight_label = NULL;
	this->param->weight = NULL;
	this->model = (svm_model*)malloc(sizeof(svm_model*));
	loadDataSet(dataFilename,numClasses,numFeatures);
	createModel();
}

IntentClassifier::IntentClassifier(string modelFilename)
{
	this->model = (svm_model*)malloc(sizeof(svm_model*));
	loadModel(modelFilename);
}

IntentClassifier::~IntentClassifier()
{
	//free(this->param);
	//free(this->model);
}

double IntentClassifier::predict(svm_node* example, bool predictProb)
{
	int svm_type=svm_get_svm_type(this->model);
	double *prob_estimates=NULL;
	double v;

	if (predictProb && (svm_type==C_SVC || svm_type==NU_SVC))
	{
		v = svm_predict_probability(this->model,example,prob_estimates, predictProb);
	}
	else
	{
		v = svm_predict(this->model,example);
	}
	return v;
}

void IntentClassifier::loadDataSet(string filename, int numClasses, int numFeatures)
{
	this->numClasses = numClasses;
	this->numFeatures = numFeatures;
	svm_problem prob = read_problem(filename.c_str());
	this->prob = prob;
	this->numExamples = prob.l;
	this->name = filename;
	this->param->gamma = (1.0/double(this->numFeatures));
}

void IntentClassifier::createModel()
{
	prob.nr_binary = error_correcting_code(param->multiclass_type,svm_find_nr_class(&this->prob),this->prob.I);
	this->model = svm_train(&this->prob,this->param);
	writeModelToFile(this->name+".model",this->model);
}

void IntentClassifier::loadModel(string filename)
{
	svm_model* m = svm_load_model(filename.c_str());
	this->model = m;
}

void IntentClassifier::writeModelToFile(string filename, svm_model* m)
{
	svm_save_model(filename.c_str(),m);
}

void IntentClassifier::setKernelFunction(int type, double degree, double gamma, double coef0)
{
	this->param->kernel_type=type;
	this->param->degree=degree;
	this->param->gamma=gamma;
	this->param->coef0 = coef0;
}

void IntentClassifier::setCostValue(double cost)
{
	this->param->C = cost;
}

void IntentClassifier::setStoppingTolerance(double epsilon)
{
	this->param->eps=epsilon;
}

