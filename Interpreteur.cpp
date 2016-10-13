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
		cout << "Principal"<< endl;
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
		cout << "Sequence"<< endl;
    sequence->ajoute(inst());
  } while (m_lecteur.getSymbole() == "<VARIABLE>" || m_lecteur.getSymbole() == "si" /*modif*/ 
          || m_lecteur.getSymbole() == "tantque" || m_lecteur.getSymbole() == "repeter"
          || m_lecteur.getSymbole() == "pour"|| m_lecteur.getSymbole() == "ecrire" 
          || m_lecteur.getSymbole() == "lire");
  // Tant que le symbole courant est un début possible d'instruction...
  // Il faut compléter cette condition chaque fois qu'on rajoute une nouvelle instruction
  return sequence;
}

Noeud* Interpreteur::inst() {
  // <inst> ::= <affectation>  ; | <instSi>
  try{
		cout << "Instruction"<< endl;
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
		cout << "Affectation"<< endl;
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
    /*m_lecteur.avancer(); faut il avancer alors qu'on avance deja apres? <---------------------------------------------------------- /!\*/
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
  // <expression> ::= <facteur> { <opBinaire> <facteur> }
  //  <opBinaire> ::= + | - | *  | / | < | > | <= | >= | == | != | et | ou
  Noeud* fact;
  try{
		cout << "Expression"<< endl;
    fact = facteur();
  }catch(SyntaxeException & e){
    m_nb_erreur++;
    cout << "Erreur de syntaxe"<< m_nb_erreur <<" : " << e.what() << endl;
    m_lecteur.avancer();
  }  
  try{
  while ( m_lecteur.getSymbole() == "+"  || m_lecteur.getSymbole() == "-"  ||
          m_lecteur.getSymbole() == "*"  || m_lecteur.getSymbole() == "/"  ||
          m_lecteur.getSymbole() == "<"  || m_lecteur.getSymbole() == "<=" ||
          m_lecteur.getSymbole() == ">"  || m_lecteur.getSymbole() == ">=" ||
          m_lecteur.getSymbole() == "==" || m_lecteur.getSymbole() == "!=" ||
          m_lecteur.getSymbole() == "et" || m_lecteur.getSymbole() == "ou"   ) {
    Symbole operateur = m_lecteur.getSymbole(); // On mémorise le symbole de l'opérateur
    m_lecteur.avancer();
    Noeud* factDroit = facteur(); // On mémorise l'opérande droit
    fact = new NoeudOperateurBinaire(operateur, fact, factDroit); // Et on construuit un noeud opérateur binaire
  }
  }catch(SyntaxeException & e){
    m_nb_erreur++;
    cout << "Erreur de syntaxe"<< m_nb_erreur <<" : " << e.what() << endl;
    m_lecteur.avancer();
  }
  //assurance de ne pas renvoyer de noeud incomplet
  if(m_nb_erreur > 0){
      fact = nullptr;
  }
  return fact; // On renvoie fact qui pointe sur la racine de l'expression
}

Noeud* Interpreteur::facteur() {
  // <facteur> ::= <entier> | <variable> | - <facteur> | non <facteur> | ( <expression> )
  Noeud* fact = nullptr;
	cout << "facteur"<< endl;
  if (m_lecteur.getSymbole() == "<VARIABLE>" || m_lecteur.getSymbole() == "<ENTIER>") {
    fact = m_table.chercheAjoute(m_lecteur.getSymbole()); // on ajoute la variable ou l'entier à la table
    m_lecteur.avancer();
  } else if (m_lecteur.getSymbole() == "-") { // - <facteur>
    m_lecteur.avancer();
    // on représente le moins unaire (- facteur) par une soustraction binaire (0 - facteur)
    fact = new NoeudOperateurBinaire(Symbole("-"), m_table.chercheAjoute(Symbole("0")), facteur());
  } else if (m_lecteur.getSymbole() == "non") { // non <facteur>
    m_lecteur.avancer();
    // on représente le moins unaire (- facteur) par une soustractin binaire (0 - facteur)
    fact = new NoeudOperateurBinaire(Symbole("non"), facteur(), nullptr);
  } else if (m_lecteur.getSymbole() == "(") { // expression parenthésée
    m_lecteur.avancer();
    fact = expression();
    testerEtAvancer(")");
  } else
    try{  
        erreur("Facteur incorrect");
    }catch(SyntaxeException & e){
        m_nb_erreur++;
        cout << "Erreur de syntaxe"<< m_nb_erreur <<" : " << e.what() << endl;
        m_lecteur.avancer();
    }
  return fact;
}

//rajouter le sinon si
Noeud* Interpreteur::instSi() {
  // <instSi> ::= si ( <expression> ) <seqInst> finsi
	
  vector<Noeud*> vectorConditions; //Tableaux de tous les noeuds de type "conditions"
  vector<Noeud*> vectorSequences; //Tableaux de tous les noeuds de type "Sequences d'instructions"
	
  try{
		cout << "si" << endl;
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
    vectorConditions.push_back(expression()); // On mémorise la condition de base
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
		cout << "sinonsi" << endl;
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
						vectorConditions.push_back(expression()); // On mémorise la condition associée a ce sinonsi
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
	cout << "sinon" << endl;
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
    
    try{
			cout << "tantQue" << endl;
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
        Noeud* condition = expression(); // On mémorise la condition
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
        Noeud* sequence = seqInst();     // On mémorise la séquence d'instruction
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
    
    //temporaire
    return nullptr;
}

Noeud* Interpreteur::instRepeter(){
    
    try{
    testerEtAvancer("repeter");
    }catch(SyntaxeException & e){
        m_nb_erreur++;
        cout << "Erreur de syntaxe"<< m_nb_erreur <<" : " << e.what() << endl;
        m_lecteur.avancer();
    }
    
    try{
    Noeud* sequence = seqInst();     // On mémorise la séquence d'instruction
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
        Noeud* expressio = expression();
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
    //temporaire
    return nullptr;
}


Noeud* Interpreteur::instPour(){
    
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
        Noeud* affect = affectation();

        
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
    Noeud* condition = expression(); //analyse de la condition
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
        Noeud* aff = affectation();

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
    Noeud* sequence = seqInst(); //analyse de la sequence d'instruction du pour
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
    //temporaire
    return nullptr;
}
Noeud* Interpreteur::instEcrire(){
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
        Noeud* var = m_table.chercheAjoute(m_lecteur.getSymbole()); // La variable est ajoutée à la table et on la mémorise
        m_lecteur.avancer();
    }
    else{ //si ce n'est pas une chaine c'est forcement une expression
        Noeud* expressi = expression();
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

        Noeud* var = m_table.chercheAjoute(m_lecteur.getSymbole()); // La variable est ajoutée à la table et on la mémorise
        m_lecteur.avancer();
        }
        else{ //si c'est autre chose qu'une chaine ou une expression une exception sera levée
            Noeud* expressio = expression();
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
    //temporaire
    return nullptr;
}
Noeud* Interpreteur::instLire(){
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
        Noeud* var = m_table.chercheAjoute(m_lecteur.getSymbole()); // on ajoute la variable à la table
        m_lecteur.avancer();
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
        Noeud* var = m_table.chercheAjoute(m_lecteur.getSymbole()); // La variable est ajoutée à la table et on la mémorise
        m_lecteur.avancer();
        cout << m_lecteur.getSymbole();
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
    //temporaire
    return nullptr;
}
