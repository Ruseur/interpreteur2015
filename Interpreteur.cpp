#include "Interpreteur.h"
#include <stdlib.h>
#include <iostream>
#include <vector>
using namespace std;

Interpreteur::Interpreteur(ifstream & fichier) :
m_lecteur(fichier), m_table(), m_arbre(nullptr),m_nb_erreur(0) {
}

void Interpreteur::analyse() {
  m_arbre = programme(); // on lance l'analyse de la première règle
}

void Interpreteur::tester(const string & symboleAttendu) const throw (SyntaxeException) {
  // Teste si le symbole courant est égal au symboleAttendu... Si non, lève une exception
  static char messageWhat[256];
  if (m_lecteur.getSymbole() != symboleAttendu) {
    sprintf(messageWhat,
            "Ligne %d, Colonne %d - Erreur de syntaxe - Symbole attendu : %s - Symbole trouvé : %s",
            m_lecteur.getLigne(), m_lecteur.getColonne(),
            symboleAttendu.c_str(), m_lecteur.getSymbole().getChaine().c_str());
    throw SyntaxeException(messageWhat);
  }
}

void Interpreteur::testerEtAvancer(const string & symboleAttendu) throw (SyntaxeException) {
  // Teste si le symbole courant est égal au symboleAttendu... Si oui, avance, Sinon, lève une exception
  tester(symboleAttendu);
  m_lecteur.avancer();
}

void Interpreteur::erreur(const string & message) const throw (SyntaxeException) {
  // Lève une exception contenant le message et le symbole courant trouvé
  // Utilisé lorsqu'il y a plusieurs symboles attendus possibles...
  static char messageWhat[256];
  sprintf(messageWhat,
          "Ligne %d, Colonne %d - Erreur de syntaxe - %s - Symbole trouvé : %s",
          m_lecteur.getLigne(), m_lecteur.getColonne(), message.c_str(), m_lecteur.getSymbole().getChaine().c_str());
  throw SyntaxeException(messageWhat);
}

Noeud* Interpreteur::programme() {
  // <programme> ::= procedure principale() <seqInst> finproc FIN_FICHIER
  Noeud* sequence = nullptr; //la déclaration de sequence n'est plus visible en dehors des blocs try-catch
  try{
    testerEtAvancer("procedure");
  }catch(SyntaxeException & e){
    m_nb_erreur++;
    cout << "Erreur de syntaxe"<< m_nb_erreur <<" : " << e.what() << endl;
    m_lecteur.avancer();
  }
  try{
    testerEtAvancer("principale");
  }catch(SyntaxeException & e){
    m_nb_erreur++;
    cout << "Erreur de syntaxe"<< m_nb_erreur <<" : " << e.what() << endl;
    m_lecteur.avancer();
  }
  try{
    testerEtAvancer("(");
  }catch(SyntaxeException & e){
    m_nb_erreur++;
    cout << "Erreur de syntaxe"<< m_nb_erreur <<" : " << e.what() << endl;
    m_lecteur.avancer();
  }
  try{
    testerEtAvancer(")");
  }catch(SyntaxeException & e){
    m_nb_erreur++;
    cout << "Erreur de syntaxe"<< m_nb_erreur <<" : " << e.what() << endl;
    m_lecteur.avancer();
  }
  try{
    sequence = seqInst();
  }catch(SyntaxeException & e){
    m_nb_erreur++;
    cout << "Erreur de syntaxe"<< m_nb_erreur <<" : " << e.what() << endl;
    m_lecteur.avancer();
  }
  try{
    testerEtAvancer("finproc");
  }catch(SyntaxeException & e){
    m_nb_erreur++;
    cout << "Erreur de syntaxe"<< m_nb_erreur <<" : " << e.what() << endl;
    m_lecteur.avancer();
  }
  try{
    tester("<FINDEFICHIER>");
  }catch(SyntaxeException & e){
    m_nb_erreur++;
    cout << "Erreur de syntaxe"<< m_nb_erreur <<" : " << e.what() << endl;
    m_lecteur.avancer();
  }  
  if (m_nb_erreur>0) {
    sequence = nullptr;
  }
	
  return sequence;
}

Noeud* Interpreteur::seqInst() {
  // <seqInst> ::= <inst> { <inst> }
  NoeudSeqInst* sequence = new NoeudSeqInst();
  do {
    sequence->ajoute(inst());
  } while (m_lecteur.getSymbole() == "<VARIABLE>" || m_lecteur.getSymbole() == "si" /*modif*/ 
          || m_lecteur.getSymbole() == "tantque" || m_lecteur.getSymbole() == "repeter"
          || m_lecteur.getSymbole() == "pour"|| m_lecteur.getSymbole() == "ecrire" 
          || m_lecteur.getSymbole() == "lire" || m_lecteur.getSymbole() == "switch" );
  // Tant que le symbole courant est un début possible d'instruction...
  // Il faut compléter cette condition chaque fois qu'on rajoute une nouvelle instruction
  return sequence;
}

Noeud* Interpreteur::inst() {
  // <inst> ::= <affectation>  ; | <instSi>
  try{
  if (m_lecteur.getSymbole() == "<VARIABLE>") {
    Noeud *affect = affectation();
    testerEtAvancer(";");
    return affect;
  }
  else if (m_lecteur.getSymbole() == "si")
    return instSi();
  else if (m_lecteur.getSymbole() == "tantque")
    return instTantQue();
  else if (m_lecteur.getSymbole() == "repeter")
    return instRepeter();
  else if (m_lecteur.getSymbole() == "pour")
    return instPour();
  else if(m_lecteur.getSymbole() == "ecrire")
    return instEcrire();
  else if(m_lecteur.getSymbole() == "lire")
      return instLire();
	else if(m_lecteur.getSymbole() == "switch")
      return instSwitch();
 
  // Compléter les alternatives chaque fois qu'on rajoute une nouvelle instruction
  else erreur("Instruction incorrecte");
  }
  catch(SyntaxeException & e){ //triage "gros grain"
      cout << e.what() << endl;
      m_lecteur.avancer();
  }
}

Noeud* Interpreteur::affectation(){
  // <affectation> ::= <variable> = <expression>
  Noeud* exp; 
  Noeud* var;
  try{
    tester("<VARIABLE>");
  }catch(SyntaxeException & e){
    
    m_nb_erreur++;
    cout << "Erreur de syntaxe"<< m_nb_erreur <<" : " << e.what() << endl;
    m_lecteur.avancer();
  }  
  try{
    var = m_table.chercheAjoute(m_lecteur.getSymbole()); // La variable est ajoutée à la table eton la mémorise
  }catch(SyntaxeException & e){
    m_nb_erreur++;
    cout << "Erreur de syntaxe"<< m_nb_erreur <<" : " << e.what() << endl;
  }  
  m_lecteur.avancer();
  try{
    testerEtAvancer("=");
  }catch(SyntaxeException & e){
    m_nb_erreur++;
    cout << "Erreur de syntaxe"<< m_nb_erreur <<" : " << e.what() << endl;
    m_lecteur.avancer();
  }  
  try{
    exp = expression();             // On mémorise l'expression trouvée
  }catch(SyntaxeException & e){
    m_nb_erreur++;
    cout << "Erreur de syntaxe"<< m_nb_erreur <<" : " << e.what() << endl;
    m_lecteur.avancer();
  }  
  Noeud* noeudaffec = new NoeudAffectation(var, exp); // On renvoie un noeud affectation
  
  //assurance de ne pas renvoyer un noeud incomplet
  if(m_nb_erreur > 0){
      noeudaffec = nullptr;
  }
  return noeudaffec;
}


Noeud* Interpreteur::expression() {
// <expression> ::= <terme> {+ <terme> |-<terme> }
    Noeud* termeGauche = terme();
    while(m_lecteur.getSymbole() == "+" || m_lecteur.getSymbole() == "-"){
        Symbole operateur = m_lecteur.getSymbole(); // On mémorise le symbole de l'opérateur
        m_lecteur.avancer();
        Noeud* termeDroit = terme();
        termeGauche = new NoeudOperateurBinaire(operateur, termeGauche, termeDroit);
    }
    return termeGauche; 
}

Noeud* Interpreteur::terme() {
    // <terme>::= <facteur> {*<facteur> |/<facteur> }
    Noeud* facteurGauche = facteur();
    while(m_lecteur.getSymbole() == "*" || m_lecteur.getSymbole() == "/"){
        Symbole operateur = m_lecteur.getSymbole(); // On mémorise le symbole de l'opérateur
        m_lecteur.avancer();
        Noeud* facteurD = facteur();
        if (facteurD == 0) throw DivParZeroException();
        facteurGauche = new NoeudOperateurBinaire(operateur, facteurGauche, facteurD);
    }
    return facteurGauche;
}

Noeud* Interpreteur::facteur() {
    // <facteur> ::= <entier> | <variable> |- <expBool> |non <expBool> |(<expBool>)
    Noeud* fact = nullptr;
    if (m_lecteur.getSymbole() == "<VARIABLE>" || m_lecteur.getSymbole() == "<ENTIER>") {
        fact = m_table.chercheAjoute(m_lecteur.getSymbole()); // on ajoute la variable ou l'entier à la table
        m_lecteur.avancer();
    } else if (m_lecteur.getSymbole() == "-") { // - <facteur>
        m_lecteur.avancer();
        // on représente le moins unaire (- facteur) par une soustraction binaire (0 - facteur)
        fact = new NoeudOperateurBinaire(Symbole("-"), m_table.chercheAjoute(Symbole("0")), expBool());
    } else if (m_lecteur.getSymbole() == "non") { // non <facteur>
        m_lecteur.avancer();
        // on représente le moins unaire (- facteur) par une soustractin binaire (0 - facteur)
        fact = new NoeudOperateurBinaire(Symbole("non"), expBool(), nullptr);
    } else if (m_lecteur.getSymbole() == "(") { // expression parenthésée
        m_lecteur.avancer();
        fact = expBool();
        testerEtAvancer(")");
    } else
        erreur("Facteur incorrect");
    
    return fact;
}


Noeud* Interpreteur::expBool() {
    // <expBool> ::=<relationET> {ou <relationEt> }
    Noeud* relationETGauche = relationET();
    while(m_lecteur.getSymbole() == "ou"){
        Symbole operateur = m_lecteur.getSymbole(); // On mémorise le symbole de l'opérateur
        m_lecteur.avancer();
        Noeud* relationETDroit = relationET();
        relationETGauche = new NoeudOperateurBinaire(operateur, relationETGauche, relationETDroit);
    }
    return relationETGauche;
}

Noeud* Interpreteur::relationET() {
    // <relationEt> ::=<relation> {et <relation> }
    Noeud* relationGauche = relation();
    while(m_lecteur.getSymbole() == "et"){
        Symbole operateur = m_lecteur.getSymbole(); // On mémorise le symbole de l'opérateur
        m_lecteur.avancer();
        Noeud* relationDroit = relation();
        relationGauche = new NoeudOperateurBinaire(operateur, relationGauche, relationDroit);
    }
    return relationGauche;
}

Noeud* Interpreteur::relation() {
    // <relation> ::= <expression> { <opRel> <expression> }
    Noeud* expressionGauche = expression();
    while(m_lecteur.getSymbole() == "==" || m_lecteur.getSymbole() == "!=" ||
          m_lecteur.getSymbole() == "<" || m_lecteur.getSymbole() == "<=" ||
          m_lecteur.getSymbole() == ">" || m_lecteur.getSymbole() == ">=" ){
        Symbole operateur = m_lecteur.getSymbole(); // On mémorise le symbole de l'opérateur
        m_lecteur.avancer();
        Noeud* expressionDroit = expression();
        expressionGauche = new NoeudOperateurBinaire(operateur, expressionGauche, expressionDroit);
    }
    return expressionGauche;
}

//rajouter le sinon si

Noeud* Interpreteur::instSi() {
  // <instSi> ::= si ( <expression> ) <seqInst> finsi
	
  vector<Noeud*> vectorConditions; //Tableaux de tous les noeuds de type "conditions"
  vector<Noeud*> vectorSequences; //Tableaux de tous les noeuds de type "Sequences d'instructions"
	
  try{
    testerEtAvancer("si");
  }catch(SyntaxeException & e){
    m_nb_erreur++;
    cout << "Erreur de syntaxe"<< m_nb_erreur <<" : " << e.what() << endl;
    m_lecteur.avancer();
  }
  try{
    testerEtAvancer("(");
  }catch(SyntaxeException & e){
    m_nb_erreur++;
    cout << "Erreur de syntaxe"<< m_nb_erreur <<" : " << e.what() << endl;
    m_lecteur.avancer();
  }
  try{
    vectorConditions.push_back(relation()); // On mémorise la condition de base
  }catch(SyntaxeException & e){
    m_nb_erreur++;
    cout << "Erreur de syntaxe"<< m_nb_erreur <<" : " << e.what() << endl;
    m_lecteur.avancer();
  }
  try{
    testerEtAvancer(")");
  }catch(SyntaxeException & e){
    m_nb_erreur++;
    cout << "Erreur de syntaxe"<< m_nb_erreur <<" : " << e.what() << endl;
    m_lecteur.avancer();
  }
  try{
    vectorSequences.push_back(seqInst());     // On mémorise la séquence d'instruction associée
  }catch(SyntaxeException & e){
    m_nb_erreur++;
    cout << "Erreur de syntaxe"<< m_nb_erreur <<" : " << e.what() << endl;
    m_lecteur.avancer();
  }
  
  try{
    while(m_lecteur.getSymbole() == "sinonsi"){ //en cas de présence de sinonsi, et tant qu'il y aura des sinonsi, on boucle
         m_lecteur.avancer();
				 
					try{
						testerEtAvancer("(");
					}catch(SyntaxeException & e){
						m_nb_erreur++;
						cout << "Erreur de syntaxe"<< m_nb_erreur <<" : " << e.what() << endl;
						m_lecteur.avancer();
					}
					try{
						vectorConditions.push_back(relation()); // On mémorise la condition associée a ce sinonsi
					}catch(SyntaxeException & e){
						m_nb_erreur++;
						cout << "Erreur de syntaxe"<< m_nb_erreur <<" : " << e.what() << endl;
						m_lecteur.avancer();
					}
					try{
						testerEtAvancer(")");
					}catch(SyntaxeException & e){
						m_nb_erreur++;
						cout << "Erreur de syntaxe"<< m_nb_erreur <<" : " << e.what() << endl;
						m_lecteur.avancer();
					}
				 
         vectorSequences.push_back(seqInst()); // On mémorise la séquence associée a ce sinon si
    }
  }catch(SyntaxeException & e){
    m_nb_erreur++;
    cout << "Erreur de syntaxe"<< m_nb_erreur <<" : " << e.what() << endl;
    m_lecteur.avancer();
  }
  
  try{
  if(m_lecteur.getSymbole() == "sinon"){
      m_lecteur.avancer();
      vectorSequences.push_back(seqInst()); // On mémorise la séquence seul représentant le "sinon"
  }
  }catch(SyntaxeException & e){
    m_nb_erreur++;
    cout << "Erreur de syntaxe"<< m_nb_erreur <<" : " << e.what() << endl;
    m_lecteur.avancer();
  }
  
  try{
    testerEtAvancer("finsi");
  }catch(SyntaxeException & e){
    m_nb_erreur++;
    cout << "Erreur de syntaxe"<< m_nb_erreur <<" : " << e.what() << endl;
    m_lecteur.avancer();
  }
	
	// On crée le noeudsi en envoyant nos 2 tableaux de conditions/séquences.
  Noeud* noeudsi = new NoeudInstSi(vectorConditions, vectorSequences);
  //assurance de ne pas envoyer un noeud incomplet
  if(m_nb_erreur > 0){
      noeudsi = nullptr;
  }

  return noeudsi; // Et on renvoie un noeud Instruction Si
}

Noeud* Interpreteur::instTantQue(){
   // <instTantQue> ::= tantque ( <expression> ) <seqInst> fintantque 
    Noeud* condition;
		Noeud* sequence;
    try{
      testerEtAvancer("tantque");
    }catch(SyntaxeException & e){
        m_nb_erreur++;
        cout << "Erreur de syntaxe"<< m_nb_erreur <<" : " << e.what() << endl;
        m_lecteur.avancer();
    }
    try{
        testerEtAvancer("(");
    }catch(SyntaxeException & e){
        m_nb_erreur++;
        cout << "Erreur de syntaxe"<< m_nb_erreur <<" : " << e.what() << endl;
        m_lecteur.avancer();
    }
    try{
        condition = relation(); // On mémorise la condition
    }catch(SyntaxeException & e){
        m_nb_erreur++;
        cout << "Erreur de syntaxe"<< m_nb_erreur <<" : " << e.what() << endl;
        m_lecteur.avancer();
    }
    try{
        testerEtAvancer(")");
    }catch(SyntaxeException & e){
        m_nb_erreur++;
        cout << "Erreur de syntaxe"<< m_nb_erreur <<" : " << e.what() << endl;
        m_lecteur.avancer();
    }
    
    try{
        sequence = seqInst();     // On mémorise la séquence d'instruction
    }catch(SyntaxeException & e){
        m_nb_erreur++;
        cout << "Erreur de syntaxe"<< m_nb_erreur <<" : " << e.what() << endl;
        m_lecteur.avancer();
    }
    
    try{
    testerEtAvancer("fintantque");
    }catch(SyntaxeException & e){
        m_nb_erreur++;
        cout << "Erreur de syntaxe"<< m_nb_erreur <<" : " << e.what() << endl;
        m_lecteur.avancer();
    }
    
  // On crée le noeudsi en envoyant la condition et la sequence d'instruction
  Noeud* noeudtq = new NoeudInstTantQue(condition, sequence);
  //assurance de ne pas envoyer un noeud incomplet
  if(m_nb_erreur > 0){
      noeudtq = nullptr;
  }

  return noeudtq; // Et on renvoie un noeud Instruction Tantque
}

Noeud* Interpreteur::instRepeter(){
	Noeud* condition;
	Noeud* sequence;
    
    try{
			testerEtAvancer("repeter");
    }catch(SyntaxeException & e){
        m_nb_erreur++;
        cout << "Erreur de syntaxe"<< m_nb_erreur <<" : " << e.what() << endl;
        m_lecteur.avancer();
    }
    
    try{
			sequence = seqInst();     // On mémorise la séquence d'instruction
    }catch(SyntaxeException & e){
        m_nb_erreur++;
        cout << "Erreur de syntaxe"<< m_nb_erreur <<" : " << e.what() << endl;
        m_lecteur.avancer();
    }
    try{
        testerEtAvancer("jusqua");
    }catch(SyntaxeException & e){
        m_nb_erreur++;
        cout << "Erreur de syntaxe"<< m_nb_erreur <<" : " << e.what() << endl;
        m_lecteur.avancer();
    }
    try{
        testerEtAvancer("(");
    }catch(SyntaxeException & e){
        m_nb_erreur++;
        cout << "Erreur de syntaxe"<< m_nb_erreur <<" : " << e.what() << endl;
        m_lecteur.avancer();
    }
    try{
        condition = relation();
    }catch(SyntaxeException & e){
        m_nb_erreur++;
        cout << "Erreur de syntaxe"<< m_nb_erreur <<" : " << e.what() << endl;
        m_lecteur.avancer();
    }
    try{
        testerEtAvancer(")");
    }catch(SyntaxeException & e){
        m_nb_erreur++;
        cout << "Erreur de syntaxe"<< m_nb_erreur <<" : " << e.what() << endl;
        m_lecteur.avancer();
    }
		
  // On crée le noeudrepeter en envoyant la condition et la sequence d'instruction
  Noeud* noeudrep = new NoeudInstRepeter(condition, sequence);
  //assurance de ne pas envoyer un noeud incomplet
  if(m_nb_erreur > 0){
      noeudrep = nullptr;
  }

  return noeudrep; // Et on renvoie un noeud Instruction Repeter
}


Noeud* Interpreteur::instPour(){
	Noeud* affectation1;
	Noeud* condition;
	Noeud* affectation2;
	Noeud* sequence;
    
    try{
      testerEtAvancer("pour");
    }catch(SyntaxeException & e){
        m_nb_erreur++;
        cout << "Erreur de syntaxe"<< m_nb_erreur <<" : " << e.what() << endl;
        m_lecteur.avancer();
    }
    try{
        testerEtAvancer("(");
    }catch(SyntaxeException & e){
        m_nb_erreur++;
        cout << "Erreur de syntaxe"<< m_nb_erreur <<" : " << e.what() << endl;
        m_lecteur.avancer();
    }
    //verifier si affectation 
    try{
			if(m_lecteur.getSymbole()=="<VARIABLE>"){
				affectation1 = affectation();
			}else{
				affectation1 = nullptr;
			}
    }catch(SyntaxeException & e){
        m_nb_erreur++;
        cout << "Erreur de syntaxe"<< m_nb_erreur <<" : " << e.what() << endl;
        m_lecteur.avancer();
    }
    try{
    testerEtAvancer(";");// 3 parties  delimitées par " ; " meme si premiere vide
    }catch(SyntaxeException & e){
        m_nb_erreur++;
        cout << "Erreur de syntaxe"<< m_nb_erreur <<" : " << e.what() << endl;
        m_lecteur.avancer();
    }
    try{
			condition = relation(); //analyse de la condition
    }catch(SyntaxeException & e){
        m_nb_erreur++;
        cout << "Erreur de syntaxe"<< m_nb_erreur <<" : " << e.what() << endl;
        m_lecteur.avancer();
    }
    try{
    testerEtAvancer(";");
    }catch(SyntaxeException & e){
        m_nb_erreur++;
        cout << "Erreur de syntaxe"<< m_nb_erreur <<" : " << e.what() << endl;
        m_lecteur.avancer();
    }
    //verifier si affectation
    try{
			if(m_lecteur.getSymbole()=="<VARIABLE>"){
				affectation2 = affectation();
			}else{
				affectation2 = nullptr;
			}
    }catch(SyntaxeException & e){
        m_nb_erreur++;
        cout << "Erreur de syntaxe"<< m_nb_erreur <<" : " << e.what() << endl;
        m_lecteur.avancer();
    }
    try{
    testerEtAvancer(")");
    }catch(SyntaxeException & e){
        m_nb_erreur++;
        cout << "Erreur de syntaxe"<< m_nb_erreur <<" : " << e.what() << endl;
        m_lecteur.avancer();
    }
    try{
			sequence = seqInst(); //analyse de la sequence d'instruction du pour
    }catch(SyntaxeException & e){
        m_nb_erreur++;
        cout << "Erreur de syntaxe"<< m_nb_erreur <<" : " << e.what() << endl;
        m_lecteur.avancer();
    }
    try{
    testerEtAvancer("finpour");
    }catch(SyntaxeException & e){
        m_nb_erreur++;
        cout << "Erreur de syntaxe"<< m_nb_erreur <<" : " << e.what() << endl;
        m_lecteur.avancer();
    }
		
    // On crée le noeudrepeter en envoyant les conditions, l'affectation et la sequence d'instruction
			Noeud* noeudPour = new NoeudInstPour(affectation1, condition, affectation2, sequence);
		//assurance de ne pas envoyer un noeud incomplet
			if(m_nb_erreur > 0){
				noeudPour = nullptr;
			}

  return noeudPour; // Et on renvoie un noeud Instruction Pour
}
Noeud* Interpreteur::instEcrire(){
	
	// On crée le noeudEcrire, que l'on va remplir à l'aide de la fonction "ajoute"
		Noeud* noeudEcrire = new NoeudInstEcrire();
					
    try{
			testerEtAvancer("ecrire");
    }catch(SyntaxeException & e){
        m_nb_erreur++;
        cout << "Erreur de syntaxe"<< m_nb_erreur <<" : " << e.what() << endl;
        m_lecteur.avancer();
    }
    try{
    testerEtAvancer("(");
    }catch(SyntaxeException & e){
        m_nb_erreur++;
        cout << "Erreur de syntaxe"<< m_nb_erreur <<" : " << e.what() << endl;
        m_lecteur.avancer();
    }
    
    try{
    if(m_lecteur.getSymbole() == "<CHAINE>"){
				Noeud* var;
        var = ( m_table.chercheAjoute(m_lecteur.getSymbole()) ); // La variable est ajoutée à la table et on la mémorise
				noeudEcrire->ajoute(var);
				
        m_lecteur.avancer();
    }
    else{ //si ce n'est pas une chaine c'est forcement une expression
        noeudEcrire->ajoute(expression());
    }
    }catch(SyntaxeException & e){
        m_nb_erreur++;
        cout << "Erreur de syntaxe"<< m_nb_erreur <<" : " << e.what() << endl;
        m_lecteur.avancer();
    }
    try{
    while(m_lecteur.getSymbole() == ","){
        
        m_lecteur.avancer();
        if(m_lecteur.getSymbole() == "<CHAINE>"){

					noeudEcrire->ajoute(m_table.chercheAjoute(m_lecteur.getSymbole()));  // La variable est ajoutée à la table et on la mémorise
					m_lecteur.avancer();
        }
        else{ //si c'est autre chose qu'une chaine ou une expression une exception sera levée
					  noeudEcrire->ajoute(expression());
        }
   
    }
    }catch(SyntaxeException & e){
        m_nb_erreur++;
        cout << "Erreur de syntaxe"<< m_nb_erreur <<" : " << e.what() << endl;
        m_lecteur.avancer();
    }
    try{
    testerEtAvancer(")");    
    }catch(SyntaxeException & e){
        m_nb_erreur++;
        cout << "Erreur de syntaxe"<< m_nb_erreur <<" : " << e.what() << endl;
        m_lecteur.avancer();
    }
	

		//assurance de ne pas envoyer un noeud incomplet
			if(m_nb_erreur > 0){
					noeudEcrire = nullptr;
			}

		return noeudEcrire; // Et on renvoie un noeud Instruction Ecrire
}
Noeud* Interpreteur::instLire(){
	
		Noeud* noeudLire = new NoeudInstLire();
	
    try{
			testerEtAvancer("lire");
    }catch(SyntaxeException & e){
        m_nb_erreur++;
        cout << "Erreur de syntaxe"<< m_nb_erreur <<" : " << e.what() << endl;
        m_lecteur.avancer();
    }
    try{
    testerEtAvancer("(");
    }catch(SyntaxeException & e){
        m_nb_erreur++;
        cout << "Erreur de syntaxe"<< m_nb_erreur <<" : " << e.what() << endl;
        m_lecteur.avancer();
    }
    
    try{
    if (m_lecteur.getSymbole() == "<VARIABLE>") {
         noeudLire->ajoute(m_table.chercheAjoute(m_lecteur.getSymbole()) ); // on ajoute la variable à la table
        m_lecteur.avancer();
    }else {
			erreur("Syntaxe incorrecte");
		}
    }catch(SyntaxeException & e){
        m_nb_erreur++;
        cout << "Erreur de syntaxe"<< m_nb_erreur <<" : " << e.what() << endl;
        m_lecteur.avancer();
    }
    
    try{
    while(m_lecteur.getSymbole() == ","){
        m_lecteur.avancer();
        
        if(m_lecteur.getSymbole() == "<VARIABLE>"){
        noeudLire->ajoute(m_table.chercheAjoute(m_lecteur.getSymbole()) ); // La variable est ajoutée à la table et on la mémorise
        m_lecteur.avancer();
        }
    }
    }catch(SyntaxeException & e){
        m_nb_erreur++;
        cout << "Erreur de syntaxe"<< m_nb_erreur <<" : " << e.what() << endl;
        m_lecteur.avancer();
    }
    try{
    testerEtAvancer(")");
    }catch(SyntaxeException & e){
        m_nb_erreur++;
        cout << "Erreur de syntaxe"<< m_nb_erreur <<" : " << e.what() << endl;
        m_lecteur.avancer();
    }
		
    //assurance de ne pas envoyer un noeud incomplet
			if(m_nb_erreur > 0){
					noeudLire = nullptr;
			}

		return noeudLire; // Et on renvoie un noeud Instruction Ecrire
}


Noeud* Interpreteur::instSwitch() {
	
  vector<Noeud*> sequences; //Tableaux de tous les noeuds de type "Sequences d'instructions"
  vector<Noeud*> entiers;   //Tableaux de tous les noeuds de type "conditions"
	Noeud* variable;
	
  try{
    testerEtAvancer("switch");
  }catch(SyntaxeException & e){
    m_nb_erreur++;
    cout << "Erreur de syntaxe"<< m_nb_erreur <<" : " << e.what() << endl;
    m_lecteur.avancer();
  }
  try{
    testerEtAvancer("(");
  }catch(SyntaxeException & e){
    m_nb_erreur++;
    cout << "Erreur de syntaxe"<< m_nb_erreur <<" : " << e.what() << endl;
    m_lecteur.avancer();
  }
  try{
		if(m_lecteur.getSymbole() == "<VARIABLE>"){
        variable = (m_table.chercheAjoute(m_lecteur.getSymbole()) ); // La variable est ajoutée à la table et on la mémorise
        m_lecteur.avancer();
    }
  }catch(SyntaxeException & e){
    m_nb_erreur++;
    cout << "Erreur de syntaxe"<< m_nb_erreur <<" : " << e.what() << endl;
    m_lecteur.avancer();
  }
  try{
    testerEtAvancer(")");
  }catch(SyntaxeException & e){
    m_nb_erreur++;
    cout << "Erreur de syntaxe"<< m_nb_erreur <<" : " << e.what() << endl;
    m_lecteur.avancer();
  }
	try{
    testerEtAvancer("{");
  }catch(SyntaxeException & e){
    m_nb_erreur++;
    cout << "Erreur de syntaxe"<< m_nb_erreur <<" : " << e.what() << endl;
    m_lecteur.avancer();
  }
  try{
		 while(m_lecteur.getSymbole() == "case"){ //en cas de présence de case, et tant qu'il y aura des case, on boucle
				m_lecteur.avancer();
				entiers.push_back(expression());
				
				try{
					testerEtAvancer(":");
				}catch(SyntaxeException & e){
					m_nb_erreur++;
					cout << "Erreur de syntaxe"<< m_nb_erreur <<" : " << e.what() << endl;
					m_lecteur.avancer();
				}
				sequences.push_back(seqInst());
		 }
  }catch(SyntaxeException & e){
    m_nb_erreur++;
    cout << "Erreur de syntaxe"<< m_nb_erreur <<" : " << e.what() << endl;
    m_lecteur.avancer();
  }
  
  
  try{
    testerEtAvancer("}");
  }catch(SyntaxeException & e){
    m_nb_erreur++;
    cout << "Erreur de syntaxe"<< m_nb_erreur <<" : " << e.what() << endl;
    m_lecteur.avancer();
  }
	
	// On crée le noeudsi en envoyant nos 2 tableaux de conditions/séquences et la variable
  Noeud* noeudswitch = new NoeudInstSwitch(entiers, sequences, variable);
  //assurance de ne pas envoyer un noeud incomplet
  if(m_nb_erreur > 0){
      noeudswitch = nullptr;
  }

  return noeudswitch; // Et on renvoie un noeud Instruction Switch
}


void Interpreteur::traduitEnCPP(ostream & cout, unsigned int indentation) const {
	
	cout <<"#include <iostream>"<< endl;
	cout <<"using namespace std;" << endl << endl;
	cout <<"int main() {" << endl; //Début du programme en c++
	
	//Je récupère chaque éléments de la table des symboles, et si c'est une variable je l'initialise.
	for(int i=0; i< this->getTable().getTaille(); i++ ) {
		SymboleValue temp = this->getTable()[i];
		
		//Test si c'est une variable
		if(temp == "<VARIABLE>") {
			cout << setw(indentation) << "" << "int " << temp.getChaine() << ";" << endl;
		}
	}
	cout << endl;
	
	getArbre()->traduitEnCPP(cout, 2+indentation);
	
	cout << endl<<setw(1*indentation)	<< "" <<"return 0;" << endl;
	cout << "}" << endl; //Fin du programme en c++	
}
