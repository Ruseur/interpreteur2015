#ifndef ARBREABSTRAIT_H
#define ARBREABSTRAIT_H

// Contient toutes les déclarations de classes nécessaires
//  pour représenter l'arbre abstrait

#include <vector>
#include <iostream>
#include <iomanip>
#include <vector>
using namespace std;

#include "Symbole.h"
#include "Exceptions.h"

////////////////////////////////////////////////////////////////////////////////
class Noeud {
// Classe abstraite dont dériveront toutes les classes servant à représenter l'arbre abstrait
// Remarque : la classe ne contient aucun constructeur
  public:
    virtual int  executer() =0 ; // Méthode pure (non implémentée) qui rend la classe abstraite
    virtual void traduitEnCPP(ostream & cout, unsigned int indentation) const { throw OperationInterditeException(); }
    virtual void ajoute(Noeud* instruction) { throw OperationInterditeException(); }
    virtual ~Noeud() {} // Présence d'un destructeur virtuel conseillée dans les classes abstraites
};

////////////////////////////////////////////////////////////////////////////////
class NoeudSeqInst : public Noeud {
// Classe pour représenter un noeud "sequence d'instruction"
//  qui a autant de fils que d'instructions dans la séquence
  public:
     NoeudSeqInst();   // Construit une séquence d'instruction vide
    ~NoeudSeqInst() {} // A cause du destructeur virtuel de la classe Noeud
    int executer();    // Exécute chaque instruction de la séquence
    void traduitEnCPP(ostream & cout, unsigned int indentation) const;
    void ajoute(Noeud* instruction);  // Ajoute une instruction à la séquence

  private:
    vector<Noeud *> m_instructions; // pour stocker les instructions de la séquence
};

////////////////////////////////////////////////////////////////////////////////
class NoeudAffectation : public Noeud {
// Classe pour représenter un noeud "affectation"
//  composé de 2 fils : la variable et l'expression qu'on lui affecte
  public:
     NoeudAffectation(Noeud* variable, Noeud* expression); // construit une affectation
    ~NoeudAffectation() {} // A cause du destructeur virtuel de la classe Noeud
    int executer();        // Exécute (évalue) l'expression et affecte sa valeur à la variable
    void traduitEnCPP(ostream & cout, unsigned int indentation) const;

  private:
    Noeud* m_variable;
    Noeud* m_expression;
};

////////////////////////////////////////////////////////////////////////////////
class NoeudOperateurBinaire : public Noeud {
// Classe pour représenter un noeud "opération binaire" composé d'un opérateur
//  et de 2 fils : l'opérande gauche et l'opérande droit
  public:
    NoeudOperateurBinaire(Symbole operateur, Noeud* operandeGauche, Noeud* operandeDroit);
    // Construit une opération binaire : operandeGauche operateur OperandeDroit
   ~NoeudOperateurBinaire() {} // A cause du destructeur virtuel de la classe Noeud
    int executer();            // Exécute (évalue) l'opération binaire)
    void traduitEnCPP(ostream & cout, unsigned int indentation) const;

  private:
    Symbole m_operateur;
    Noeud*  m_operandeGauche;
    Noeud*  m_operandeDroit;
};

////////////////////////////////sinon si Damien ////////////////////////////////////////////////
class NoeudInstSi : public Noeud {
// Classe pour représenter un noeud "instruction si"
//  et ses 2 fils : la condition du si et la séquence d'instruction associée
  public:
    NoeudInstSi(vector<Noeud*> condition, vector<Noeud*> sequence);
     // Construit une "instruction si" avec sa condition et sa séquence d'instruction
   ~NoeudInstSi() {} // A cause du destructeur virtuel de la classe Noeud
    int executer();  // Exécute l'instruction si : si condition vraie on exécute la séquence
    void traduitEnCPP(ostream & cout, unsigned int indentation) const;

  private:
    vector<Noeud*>  m_condition;
    vector<Noeud*>  m_sequence;
};


////////////////////////////  Damien  ////////////////////////////////////////////////////
class NoeudInstTantQue : public Noeud {
// Classe pour représenter un noeud "instruction tantque"
//  et ses 2 fils : la condition du tantque et la séquence d'instruction associée
  public:
    NoeudInstTantQue(Noeud* condition, Noeud* sequence);
     // Construit une "instruction tantque" avec sa condition et sa séquence d'instruction
   ~NoeudInstTantQue() {} // A cause du destructeur virtuel de la classe Noeud
    int executer();  // Exécute l'instruction tantque : tantque condition vraie on exécute la séquence
    void traduitEnCPP(ostream & cout, unsigned int indentation) const;

  private:
    Noeud*  m_condition;
    Noeud*  m_sequence;
};

////////////////////////////  Damien  ////////////////////////////////////////////////////
class NoeudInstRepeter : public Noeud {
// Classe pour représenter un noeud "instruction Repeter"
//  et ses 2 fils : la condition du Repeter et la séquence d'instruction associée
  public:
    NoeudInstRepeter(Noeud* condition, Noeud* sequence);
     // Construit une "instruction Repeter" avec sa condition et sa séquence d'instruction
   ~NoeudInstRepeter() {} // A cause du destructeur virtuel de la classe Noeud
    int executer();  // Exécute l'instruction Repeter : Repeter condition vraie on exécute la séquence
    void traduitEnCPP(ostream & cout, unsigned int indentation) const;

  private:
    Noeud*  m_condition;
    Noeud*  m_sequence;
};

////////////////////////////  Damien  ////////////////////////////////////////////////////
class NoeudInstPour : public Noeud {
// Classe pour représenter un noeud "instruction Pour "
//  et ses 4 fils : l'affectation, expression, affectation et sa séquence d'instruction associée
  public:
    NoeudInstPour(Noeud* affectation1, Noeud* condition, Noeud* affectation2, Noeud* sequence);
     // Construit une "instruction Pour" avec ses affectations et expression et sa séquence d'instruction
   ~NoeudInstPour() {} // A cause du destructeur virtuel de la classe Noeud
    int executer();  // Exécute l'instruction Pour : Pour (affectation, expression, affectation) { sequence d'instructions
    void traduitEnCPP(ostream & cout, unsigned int indentation) const;

  private:
    Noeud*  m_affectation1;
    Noeud*  m_affectation2;
    Noeud*  m_condition;
    Noeud*  m_sequence;
};


////////////////////////////  Damien  ////////////////////////////////////////////////////
class NoeudInstEcrire : public Noeud {
// Classe pour représenter un noeud "instruction Ecrire "
//  et son fils : un tableau d'expression à écrire
  public:
    NoeudInstEcrire(); // Construit une "instruction Ecrire" avec son tableau d'expressions/chaines
   ~NoeudInstEcrire() {} // A cause du destructeur virtuel de la classe Noeud
    int executer();  // Exécute l'instruction écrire : écrit la expressions/chaines
    void ajoute(Noeud* expression);  // Ajoute une expression/chaine à la séquence
    void traduitEnCPP(ostream & cout, unsigned int indentation) const;

  private:
    vector<Noeud *> m_expressions; // pour stocker les expressions/chaines à écrire
};


////////////////////////////  Damien  ////////////////////////////////////////////////////
class NoeudInstLire : public Noeud {
// Classe pour représenter un noeud "instruction Lire "
//  et son fils : un tableau de variable dont on doit lire les valeurs et leurs affecter
  public:
    NoeudInstLire(); // Construit une "instruction Lire" avec son tableau de variables
   ~NoeudInstLire() {} // A cause du destructeur virtuel de la classe Noeud
    int executer();  // Exécute l'instruction lire : lire les valeurs et les affectes aux variables associées
    void ajoute(Noeud* variable);  // Ajoute une variable à la séquence
    void traduitEnCPP(ostream & cout, unsigned int indentation) const;

  private:
    vector<Noeud *> m_variables; // pour stocker les variables à lire
};

//////////////////////////// Damien /////////////////////////////////////////////////////
class NoeudInstSwitch : public Noeud {
    
  public:
    NoeudInstSwitch(vector<Noeud*> ent, vector<Noeud*> seq, Noeud* var); // Construit une "instruction Switch" 
   ~NoeudInstSwitch() {} // A cause du destructeur virtuel de la classe 
    int executer();  // Exécute l'instruction switch : executer le morceau de code associé à la bonne condition
    void traduitEnCPP(ostream & cout, unsigned int indentation) const;

  private:
    vector<Noeud*> m_entiers;
    vector<Noeud*> m_sequence;
    Noeud* m_variable; 
    
};

#endif /* ARBREABSTRAIT_H */
