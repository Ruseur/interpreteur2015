#include <stdlib.h>
#include "ArbreAbstrait.h"
#include "Symbole.h"
#include "SymboleValue.h"
#include "Exceptions.h"

////////////////////////////////////////////////////////////////////////////////
// NoeudSeqInst
////////////////////////////////////////////////////////////////////////////////

NoeudSeqInst::NoeudSeqInst() : m_instructions() {
}
int NoeudSeqInst::executer() {
	cout << "executage sequence" << endl;
  for (unsigned int i = 0; i < m_instructions.size(); i++)
    m_instructions[i]->executer(); // on exécute chaque instruction de la séquence
  return 0; // La valeur renvoyée ne représente rien !
}

void NoeudSeqInst::ajoute(Noeud* instruction) {
  if (instruction!=nullptr) m_instructions.push_back(instruction);
}

////////////////////////////////////////////////////////////////////////////////
// NoeudAffectation
////////////////////////////////////////////////////////////////////////////////

NoeudAffectation::NoeudAffectation(Noeud* variable, Noeud* expression)
: m_variable(variable), m_expression(expression) {
}

int NoeudAffectation::executer() {
	cout << "executage affectation" << endl;
  int valeur = m_expression->executer(); // On exécute (évalue) l'expression
  ((SymboleValue*) m_variable)->setValeur(valeur); // On affecte la variable
  return 0; // La valeur renvoyée ne représente rien !
}

////////////////////////////////////////////////////////////////////////////////
// NoeudOperateurBinaire
////////////////////////////////////////////////////////////////////////////////

NoeudOperateurBinaire::NoeudOperateurBinaire(Symbole operateur, Noeud* operandeGauche, Noeud* operandeDroit)
: m_operateur(operateur), m_operandeGauche(operandeGauche), m_operandeDroit(operandeDroit) {
}

int NoeudOperateurBinaire::executer() {
	cout << "executage operateur binaire" << endl;
  int og, od, valeur;
  if (m_operandeGauche != nullptr) og = m_operandeGauche->executer(); // On évalue l'opérande gauche
  if (m_operandeDroit != nullptr) od = m_operandeDroit->executer(); // On évalue l'opérande droit
  // Et on combine les deux opérandes en fonctions de l'opérateur
  if (this->m_operateur == "+") valeur = (og + od);
  else if (this->m_operateur == "-") valeur = (og - od);
  else if (this->m_operateur == "*") valeur = (og * od);
  else if (this->m_operateur == "==") valeur = (og == od);
  else if (this->m_operateur == "!=") valeur = (og != od);
  else if (this->m_operateur == "<") valeur = (og < od);
  else if (this->m_operateur == ">") valeur = (og > od);
  else if (this->m_operateur == "<=") valeur = (og <= od);
  else if (this->m_operateur == ">=") valeur = (og >= od);
  else if (this->m_operateur == "et") valeur = (og && od);
  else if (this->m_operateur == "ou") valeur = (og || od);
  else if (this->m_operateur == "non") valeur = (!og);
  else if (this->m_operateur == "/") {
    if (od == 0) throw DivParZeroException();
    valeur = og / od;
  }
  return valeur; // On retourne la valeur calculée
}

////////////////////////////////////////////////////////////////////////////////
// NoeudInstSi
////////////////////////////////////////////////////////////////////////////////

NoeudInstSi::NoeudInstSi(vector<Noeud*> conditions, vector<Noeud*> sequences)
: m_condition(conditions), m_sequence(sequences) {
}

int NoeudInstSi::executer() {
	cout << "executage si" << endl;
	
	int i=0; // indice pour déterminer quel séquence executer
	bool termine = false; //afin de sortir de la loop en cas de conditions TRUE
	
	// On loop tant qu'il reste des conditions non TRUE
	while(termine == false && i< m_condition.size()) {
		if(m_condition[i]->executer()) { //On teste la condition de l'indice i
			termine = true;
			i = i-1; //Je réduis l'indice de 1 si la condition est bonne afin d'executer la séquence associée à ce sinon si
		}
		i++;
	}
	if(i<m_sequence.size()) { // Je vérifie si, même si aucune condition n'est respectée, il reste une séquence qui est le "sinon"
		m_sequence[i]->executer();
	}
	
  return 0; // La valeur renvoyée ne représente rien !
}



////////////////////////////////////////////////////////////////////////////////
// NoeudInstTantQue
////////////////////////////////////////////////////////////////////////////////

NoeudInstTantQue::NoeudInstTantQue(Noeud* condition, Noeud* sequence)
: m_condition(condition), m_sequence(sequence) {
}

int NoeudInstTantQue::executer() {
	while(m_condition->executer()) {
		m_sequence->executer();
	}
  return 0; // La valeur renvoyée ne représente rien !
}

////////////////////////////////////////////////////////////////////////////////
// NoeudInstRepeter
////////////////////////////////////////////////////////////////////////////////

NoeudInstRepeter::NoeudInstRepeter(Noeud* condition, Noeud* sequence)
: m_condition(condition), m_sequence(sequence) {
}

int NoeudInstRepeter::executer() {
	do {
		m_sequence->executer();
	}
	while(m_condition->executer());
	
  return 0; // La valeur renvoyée ne représente rien !
}


////////////////////////////////////////////////////////////////////////////////
// NoeudInstPour
////////////////////////////////////////////////////////////////////////////////

NoeudInstPour::NoeudInstPour(Noeud* affectation1, Noeud* condition, Noeud* affectation2, Noeud* sequence)
: m_affectation1(affectation1), m_condition(condition),m_affectation2(affectation2), m_sequence(sequence) {
}

int NoeudInstPour::executer() {
	for(m_affectation1->executer(); m_condition->executer(); m_affectation2->executer() ) {

			m_sequence->executer();
	}
	
  return 0; // La valeur renvoyée ne représente rien !
}


////////////////////////////////////////////////////////////////////////////////
// NoeudInstEcrire
////////////////////////////////////////////////////////////////////////////////

NoeudInstEcrire::NoeudInstEcrire() {
}

int NoeudInstEcrire::executer() {

	for(Noeud* temp : m_expressions) {
		cout << temp->executer();
	}
	
  return 0; // La valeur renvoyée ne représente rien !
}

void NoeudInstEcrire::ajoute(Noeud* expression) {
  if (expression!=nullptr) m_expressions.push_back(expression);
}



////////////////////////////////////////////////////////////////////////////////
// NoeudInstLire
////////////////////////////////////////////////////////////////////////////////

NoeudInstLire::NoeudInstLire() {
}

int NoeudInstLire::executer() {

	for(Noeud* temp : m_variables) {
		int temp2;
		cin >> temp2;

		((SymboleValue*) temp)->setValeur(temp2);
	}
	
  return 0; // La valeur renvoyée ne représente rien !
}

void NoeudInstLire::ajoute(Noeud* variable) {
  if (variable!=nullptr) m_variables.push_back(variable);
}

