Δημήτριος Σιταράς	|
1115201800178 	 	|
———————————————————


► Οργάνωση Κώδικα:
  .
  ├── client
  │   ├── clientUtilities
  │   │   ├── clientUtilities.c
  │   │   └── clientUtilities.h
  │   └── remoteClient.c
  ├── dataServerUtilities
  │   ├── dataServerUtilities.c -----> communicationThread()
  │   └── dataServerUtilities.h
  ├── filesQueue
  │   ├── filesQueue.c
  │   └── filesQueue.h
  ├── hashTableClients
  │   ├── hashTableClients.c
  │   ├── hashTableClients.h
  │   └── hashTableListClients
  │       ├── hashTableListClients.c
  │       └── hashTableListClients.h
  ├──threadPool
  │   ├── threadPool.c -----> workerThread()
  │   └── threadPool.h
  ├── Makefile
  ├── README.txt
  └── dataServer.c


► Γενικά:

  → Ο κώδικας είναι σχολιασμένος.

  → Πληρούνται όλες οι προϋποθέσεις/απαιτήσεις που αναγράφονται στην εκφώνηση της εργασίας.

  → Γίνεται το απαραίτητο Εrror Ηandling.

  → Έχω χρησιμοποιήσει κώδικα από τις διαφάνειες του μαθήματος (κυρίως από το pdf "topic5-Sockets", προκειμένου να υλοποιήσω το client-server model ).

  → Όλη η μνήμη που δεσμεύεται δυναμικά κατά την εκτέλεση του προγράμματος, αποδεσμεύεται πλήρως
    ( Έχει ελεγχθεί μέσω valgrind στα μηχανήματα linux της σχολής).

  → Eντολή μεταγλώττισης: make (υπάρχει αρχείο Makefile)

  → Εντολή εκτέλεσης του Server: ./dataServer -p <port> -s <thread_pool_size> -q <queue_size> -b <block_size>

      p.ex.: ./dataServer -p 8000 -s 2 -q 2 -b 20

  → Εντολή εκτέλεσης του Client: ./remoteClient -i <server_ip> -p <server_port> -d <directory>

      p.ex.: ./remoteClient -i linux04 -p 8000 -d server

  → make clean, για την διαγραφή των παραγόμενων από την μεταγλώττιση αρχείων.

  → Ο τερματισμός του server και των clients του γίνεται πατατώντας CTRL-C ή στέλνοντας SIGINT στον server
    ( ο τερματισμός είναι σημαντικό να γίνεται μόνο όταν ο server έχει ολοκληρώσει όλες τις αποστολές αρχείων ).


► Πρωτόκολλο Επικοινωνίας & Τερματισμός Server και των αντίστοιχων Clients

  → Κάθε ένα worker thread του server πρωτού στείλει το αρχείο που του έχει ανατεθεί στον αντίστοιχο client, στέλνει (μέσω της write()) έναν ακέραιο με τιμή ίση με 0 (endOfCom),
    έτσι ώστε στην συνέχεια ο client να είναι "έτοιμος" να λάβει ένα αρχείο απο τον server (που περιέχει ο φάκελος που ζήτησε από τον server).

  → Έχοντας υλοποιήσει signal handler για το SIGINT, o τερματισμός του server και των clients του γίνεται με CTRL-C (SIGINT) στον server.
    Συγκεκριμένα, μόλις ο server λαβει το SIGINT, σε πρώτη φάση τερματίζει όλα τα worker threads και αποδεσμεύει την Thread Pool δομή (που έχει γίνει προηγουμένως allocate),
    στην συνέχεια στέλνει σε κάθε client (μέσω της write()) έναν ακέραιο με τιμή ίση με 1 (endOfCom) σηματοδοτώντας έτσι τον τερματισμό την επικοινωνίας, ως επακόλουθο ο client
    να τερματίζει. Τέλος, ο server κλείνει όλα τα connections (δηλαδή όλα τα file descriptors που επέστρεψε η accept()) και τερματίζει.


► Αποστολή και Διάβασμα των περιεχομένων ενός αρχείου

  → Server - Worker Thread - dataServerUtilities/dataServerUtilities.c ---> sendFileData()

    Για την αποστολή των περιεχομένων ενός αρχείου αρχικά στέλνω το μέγεθος του.
    Στην συνέχεια, με μια while() διαβάζω το αρχείο και το αποστέλλω στον client ανά μπλοκ, το κάθε μπλοκ έχει μέγεθος το πολύ block size bytes (όρισμα από το command line: -b <block_size>).
    Όταν, το file data είναι μεγαλύτερο από το block size τότε όλα τα "μπλοκς", εκτός του τελευταίου,
    είναι μεγέθους block size bytes, ενώ το τελευταίο ενδεχομένως να είναι λιγότερο από block size bytes.

  → Client - client/clientUtilities/clientUtilities.c ---> readFileData()

    Αντίστοιχα, για το διάβασμα των περιεχομένων ενός αρχείου αρχικά διαβάζω το μέγεθος αυτού.
    Έπειτα, ομοίως, με μια while() διαβάζω το file data που στέλνεται από τον server ανά "μπλοκς" γνωρίζοντας μέσω του file size το μέγεθος του επόμενου "μπλοκ" που πρόκειται να διαβάσω.
    Έτσι, ενώνοντας τα μπλοκς αυτά (με την memcpy()) "σχηματίζω" τελικά το data του αντίστοιχου αρχείου.


► Δομές Δεδομένων

    Υλοποιήσα και χρησιμοποίησα τις παρακάτω δομές δεδομένων:

    → Ουρά (Queue) της οποίας η υλοποίηση βρίσκεται στον φάκελο filesQueue/ και είναι ίδια με αυτής της προηγούμενης εργασίας
      μόνο που έχω προσθέσει την μεταβλητή maxNumberOfNodes στην δομή και έναν έλεγχο στην συνάρτηση pushQueue(), προκειμένου να δέχεται έναν συγκεκριμένο
      αριθμό θέσεων/κόμβων. Έτσι, όταν η ουρά ειναι γεμάτη, δηλαδή ο αριθμός των κόμβων της ισούται με τον αριθμό των θέσεων
      που μπορεί να έχει (numberOfNodes == maxNumberOfNodes), και κληθεί η συνάρτηση pushQueue() τότε δεν γίνεται push νέος κόμβος
      στην ουρά και η συνάρτηση επιστρέφει 1 (return 1;).

    → Έναν πίνακα κατακερματισμού (hash table) του οποίου η υλοποίηση βρίσκεται στον φάκελο hashTableClients/ και είναι ίδια με αυτής της προηγούμενης εργασίας.
      Χρησιμοποιείται στον Server και το προσπευλάνουν το main thread (insert) και τα worker threads (search).
      Σε αυτό αποθηκεύω πληροφορίες για κάθε έναν client, συγκεκριμένα :

            1) το file descriptor του αντίστοιχου client ( δηλαδή αυτό που επιστρέφει η συνάρτηση accept() ),
            2) το Id του communication thread, το οποίο "αναλαμβάνει" τον συγκεκριμένο client,
            3) το "όνομα" του αντίστοιχου client (δηλαδή αυτό που επιστρέφει η συνάρτηση gethostbyaddr()->h_name),
            4) τέλος ορίζεται και αρχικοποιείται και ένα Mutex (mtxClient) για τον αντίστοιχο client,
               το οποίο χρησιμοποιείται για να διασφαλιστεί ότι στο socket του αντίστοιχου client γράφει δεδομένα
               μόνο ένα worker thread τη φορά.

      Σημείωση: επειδή το hash table αποτελεί έναν κοινόχρηστο πόρο (όπως και η ουρά), χρησιμοποιώ το binary mutex htMtx καθώς και το condition variable nonClient
                προκειμένου το main thread (που κάνει insert) και τα worker threads (που κάνουν search) να προσπευλάνουν το hash table συγχρονισμένα, 
                χωρίς να παρατηρούνται "ασυνέπειες" λόγω race condition. Ειδικότερα, "αποφεύγω" την περίπτωση στην οποία το main thread "δεν έχει προλάβει" να κάνει insert
                τις πληροφορίες για τον αντίστοιχο client που κάνει search ένα worker thread. Έτσι λοιπόν, αν δεν υπάρχει το αντίστοιχο node στο hash table, που κάνει search ένα worker thread,
                τότε μπλοκάρει εκεί (pthread_cond_wait()) μέχρις ότου γίνει insert από το main thread (στο main thread χρησιμοποιώ pthread_cond_broadcast() καθώς μπορεί να έχουν μπλοκαριστεί πολλά worker threads).


► Συγχρονισμός μεταξύ των communication και των worker threads (όσον αναφορά την προσπέλαση της ουράς)

  Ο συγχρονισμός μεταξύ των communication και των worker threads όσον αναφορά την προσπέλαση της ουράς (push και pop αντίστοιχα)
  επιτέυχθηκε με ένα mutex (queueMtx) και δύο condition variables (nonEmpty και nonFull). Αναλυτικότερα:

    → Επειδή τα communication και τα worker threads επιχειρούν ταυτόχρονα push και pop αντίστοιχα στην "κοινόχρηστη" ουρά (queue), χρησιμοποιώ το binary mutex queueMtx.
      Συνεπώς, με την χρήση αυτού του mutex, και σε συνδιασμό βέβαια με τα condition variables που αναλύονται ακολούθως, τα παραπάνω threads προσπευλάνουν τον κοινόχρηστο πόρο
      συγχρονισμένα, χωρίς να παρατηρούνται "ασυνέπειες" λόγω race condition.

      Condition Variables:

        → Worker Thread:  κάθε ένα worker thread χρησιμοποιώντας το condition variable nonEmpty ( και ένα βρόγχο while() ) ελέγχει αν η ουρά (queue) είναι άδεια (και αν το flag για την διαγραφή του thread pool ειναι false).
                          Έαν είναι άδεια η ουρά, τότε μπλοκάρει εκεί (pthread_cond_wait()) μέχρις ότου γίνει push ένα (τουλάχιστον) file path από ένα
                          communication thread, λαμβάντοντας έτσι signal (μέσω της pthread_cond_signal()) στο condition variable nonEmpty.
                          Σημείωνω πως σε κατά την έναρξη του server τα worker threads που δημιουργούνται μπλοκάρουν εκεί, όπως είναι λογικό, καθώς η ουρά αρχικά ειναι άδεια, διότι κανένας client δεν έχει συνδεθεί
                          ακόμα με τον server, επομένως κανένα communication thread δεν εχει δημιουργηθεί ακόμα ώστε να κάνει push file paths στην ουρά.
                          Διαφορετικά, εάν δεν είναι άδεια η ουρά, τότε γίνεται pop από αυτή, ώστε το αντίστοιχο worker thread να πάρει το node που περιέχει ένα file path και το file descriptor του αντίστοιχου client που πρέπει να στείλει το αρχείο.
                          Έτσι, στη συνέχεια, αφού κάνει Lock το mtxClient του αντίστοιχου client, ώστε να να διασφαλιστεί ότι στο socket του γράφει δεδομένα μόνο το συγκεκριμένο worker thread, στέλνει το αρχείο στον
                          client και ακουλούθως (αφού κάνει Unlock το mtxClient του αντίστοιχου client) ξεκινάει την διαδικασία πάλι από την αρχή.

        → Communication Thread:  αντίστοιχα κάθε ένα communication thread χρησιμοποιώντας το condition variable nonFull ( και ένα βρόγχο while() ) ελέγχει αν η ουρά (queue) είναι γεμάτη.
                                 Έαν είναι γεμάτη η ουρά, τότε μπλοκάρει εκεί (pthread_cond_wait()) μέχρις ότου γίνει pop ένα (τουλάχιστον) file path από ένα
                                 worker thread λαμβάντοντας έτσι signal (μέσω της pthread_cond_signal()) στο condition variable nonFull.
                                 Εάν δεν είναι γεμάτη η ουρά, τότε κάνει ένα push σε αυτή file paths και τους file descriptors του αντίστοιχου κάθε φορά client στον οποίο ακολούθως
                                 θα στείλει το αρχείο το worker thread.


► Σημείωσεις

  → Κάθε ένα communication thread τερματίζει (pthread_exit(NULL)) μόλις βάλει στην ουρά όλα τα αρχεία που περιέχει ο ζητούμενος φάκελος από τον αντίστοιχο client.

  → Για τον τερματισμό του server και των clients πατάμε CTRL-C ή στέλνουμε SIGINT στον server ΜΟΝΟ όταν ο server έχει ολοκληρώσει όλες τις αποστολές αρχείων προς τους αντίστοιχους clients
    (δηλαδή όταν ΟΛΑ τα αρχεία που περιέχονται στους ζητούμενους καταλόγους γίνουν received από τους αντίστοιχους clients).

  → Ο συγχρονισμός μεταξύ των communication και των worker threads βασίστηκε στο 2o παράδειγμα producer-consumer των διαφανειών του μαθήματος.

  → H macro function perror2() που χρησιμοποιείται για Εrror Ηandling είναι από τις διαφάνειες του μαθήματος.

  → Το hash table αρχικοποιείται με 15 buckets.

  → Το max length που μπορεί να έχει ένα file path είναι 4096, επομένως προκειμένου να το αποθηκεύσω ορίζω έναν στατικό πίνακα μεγέθους 4096 (char path[2048] στην συνάρτηση extractDirectoryContents() του αρχείου dataServerUtilities/dataServerUtilities.c).
