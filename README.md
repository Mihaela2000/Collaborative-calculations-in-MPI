Calcule colaborative in sisteme distribuite

  In acest proiect, am implementat un program distribuit in MPI in care 
  procesele sunt grupate intr-o topologie formata din trei clustre, fiecare 
  din acestea avand cate un coordonator si cate un numar arbitrar de procese 
  worker. Procesele worker dintr-un cluster pot sa comunice doar cu 
  coordonatorul lor, iar toti cei trei coordonatori pot sa comunice intre ei 
  pentru a conecta clustrele.

  Scopul proiectului este ca toate procesele worker sa ajunga sa lucreze 
  impreuna, cu ajutorul coordonatorilor, pentru rezolvarea unor task-uri 
  computationale. Acest lucru se va realiza prin stabilirea topologiei si 
  diseminarea ei catre toate procesele, si apoi prin impartirea calculelor 
  in mod cat mai echilibrat intre workeri.


Task 1 - Stabilirea topologiei
  coordinators   - matrice ce are pe fiecare linie rank-urile procesele 
                   fiecarui coordonator
  coordinator    - variabile ce tine rank-ul fiecarui coordonator
  process_worker - vector ce contine numarul de workeri ai fiecarui 
                   coordonator

  La inceputul executiei programului distribuit, procesele coordonator vor 
  citi informatii despre procesele din clusterele lor din trei fisiere de 
  intrare, apoi isi transmit intre ei informatiile. Dupa ce fiecare 
  coordonator stie topologia, acestia o transmit mai departe workerilor. 
  Cu functia print_topology, afisez topologia pentru fiecare coordonator si 
  worker.


Task 2 - Realizarea calculelor
  Odata ce toate procesele cunosc topologia, urmeaza partea de calcule, care 
  este coordonata de catre procesul 0. Procesul 0 genereaza vectorul v si 
  distribuie mai departe catre ceilalti coordonatori. Fiecare coordonator 
  trimite catre workeri vectorul si partea din vector care trebuie inmultita 
  cu 2 de fiecare worker. Dupa ce workerii si-au terminat treaba, trimit 
  bucata coordonatorilor. Apoi fiecare coordonator trimite partea sa, 
  coordonatorului 0. La final, procesul 0 afiseaza vectorul rezultat.
