#include <iostream>
#include <string>
#include <cstdlib>
#include <getopt.h>
#include <sys/time.h>
#include "csrc.h"

using std::cout;
using std::cerr;


// Struttura utilizzata per raccogliere le opzioni da riga di comando
struct SRCoption {
	int SRCaction;
	bool random_samples, random_theta,  do_sync, binary, iso, sys_sync;
	int verb, chdate, leap, fc, channels, timeout, wds, repeat, dst;
	double th, power, noise, snr_level;
	long delay;
	char *soundDev, *fo, *logfile, *setDate;
};




void print_help(const char* filename)
{
  cout<<"Utilizzo:\t" <<filename <<" --decode [OPZIONI]\n"
	<<"oppure\t" <<filename <<" --play [OPZIONI]\n"
	<<"\nLista delle opzioni disponibili:\n"
	<<"  -d, --decode\t\tDecodifica il segnale SRC\n"
	<<"  -y, --system-sync\tSincronizza l'ora di sistema con il segnale SRC. Necessita\n\t\t\t dei privilegi di super user\n"
	<<"  -N, --snr=SNR_LEVEL\tLivello di rilevamento SNR sul rumore in dB.\n\t\t\tDi default è SNR_LEVEL=5dB sopra il livello di rumore\n"
	<<"  -W, --window=LENGTH\tWindow Decision System. Imposta la lunghezza della finestra\n\t\t\tin time symbols. Di default è LENGTH=50 simboli\n"
	<<"  -D, --delay=DELAY\tdelay della sincronizzazione in microsecondi\n"
	<<"  -T, --timeout=TIMEOUT\tImposta il timeout per la decodifica in secondi\n"
	<<"  -t, --threshold=TH\tImposta la soglia di decisione statica in dB(default -35 dB)\n"
	<<'\n'
	<<"  -p, --play\t\tRiproduci il segnale SRC\n"
	<<"  -k, --rand-theta\tDa un theta random in riproduzione\n"
	<<"  -o, --rand-samples\tAggiungi dei campioni random (< 1 sec) prima di riprodurre il segnale\n"
	<<"  -a, --power=POWER\tLivello di potenza del segnale in dB in riproduzione\n"
	<<"  -n, --noise=SIGMA\tRiproduce il segnale SRC con un rumore espresso come il RMS\n"
	<<"  -S, --set-date=DATE\tImposta la data/ora del segnale SRC nel formato <hh:mm DD/MM/YYYY>\n"
	<<"  -e, --dst-on\t\tForza il flag per l'ora solare (OE) ad essere 1\n"
	<<"  -E, --dst-off\t\tForza il flag per l'ora solare (OE) ad essere 0\n"
	<<"  -C, --change-time=SE\tImposta i giorni di avviso SRC per il cambiamento dell'ora legale/solare [0-7]\n\t\t\t(7 = senza avviso, default)\n"
	<<"  -l, --leap-second=SI\tImposta l'avviso SRC del secondo intercalare (+1|-1)\n\t\t\talla fine del mese\n"
	<<'\n'
	<<"  -s, --sync\t\tSincronizzazione con gli impulsi sonori da 1kHz\n\n"
	<<"  -r, --rate=FREQ\tImposta la frequenza di campionamento (per l'input/output su file\n\t\t\tdi default è 8 kHz)\n"
	<<"  -m, --mono\t\tImposta l'audio in mono (1 canale). Di default è stereo\n\t\t\t(2 canali)\n"
	<<"  -M, --stero\t\tImposta l'audio in stereo (2 canali). Questa è\n\t\t\tl'opzione di default\n"
	<<"  -f, --file=FILE\tSeleziona il file sorgente/destinazione (l'impostazione predefinita è il server audio)\n"
	<<"  -c, --card=DEV\tSpecifica un diverso dispositivo audio\n"
	<<"  -v, --debug=LEVEL\tLivello verbose (default = 1)\n"
	<<"  -I, --iso\t\tStampa la data/ora nel formato ISO 8601\n\t\t\t(predefinito: formato RFC2822)\n"
	<<"  -b, --binary\t\tStampa la rappresentazione binaria del segnale SRC\n"
	<<"  -R, --repeat=TIMES\tNumero di ripetizioni di decodifica (predefinito = 1.\n\t\t\tImposta 0 per ripetizioni illimitate)\n"
	<<"  -L, --logfile=LOG\tReindirizzare le uscite al registro file\n"
	<<"  -w, --warranty\tDettagli sulla garanzia\n"
	<<"  -V, --version\t\tVersione del programma\n"
	<<"  -h, --help\t\tStampa nuovamente questo aiuto e poi esce\n\n";
}



void print_warranty()
{
  cout <<"Questo programma viene fornito senza alcuna garanzia\n"
       <<"Questo è software libero, e sei invitato a modificarlo, eseguirlo e ridistribuirlo\n"
			 <<"Questo è software è sotto licenza GPL v3\n";
}



int main(int argc, char **argv) {

  Csrc SRC;
  SRCoption options;
  const char* VERSION = "1.1";
  int error = 0;
  int choice;

  bool openState;	// used for error control


  // default values:
  options.random_samples = false;
  options.random_theta = false;
  options.do_sync = false;
  options.binary = false;
  options.iso = false;
  options.sys_sync = false;
  options.dst = 0;		// default, set by system date
  options.verb = 1;		// normal verbose level
  options.chdate = 7;		// no change date
  options.leap = 0;
  options.fc = 8000;		// 8 kHz
  options.channels = 2;		// stereo, 2 channels
  options.th = -35;
  options.power = -6.0;
  options.noise = 0.0;
  options.timeout = 0;		// uses default
  options.wds = 50;
  options.snr_level = 5.0;	// 5 dB SNR default

  options.fo = NULL;
  options.soundDev = NULL;	// default sound device
  options.logfile = NULL;
  options.setDate = NULL;
  options.repeat = 1;
  options.SRCaction = 0;
  options.delay = 1;



  if(argc == 1) {
    cerr <<"Errore: è richiesta almeno un'opzione\n\n";
    print_warranty();
    print_help(argv[0]);
    return 1;
  }

  int longindex = 0;	// variable used for the getopt_long function
  static struct option long_options[] = {
		{"decode",       no_argument,       NULL, 'd'},
		{"play",         no_argument,       NULL, 'p'},
		{"system-sync",  no_argument,       NULL, 'y'},
		{"delay",        required_argument, NULL, 'D'},
		{"rand-theta",   no_argument,       NULL, 'k'},
		{"rand-samples", no_argument,       NULL, 'o'},
		{"set-date",     required_argument, NULL, 'S'},
		{"dst-on",       no_argument,       NULL, 'e'},
		{"dst-off",      no_argument,       NULL, 'E'},
		{"power",        required_argument, NULL, 'a'},
		{"sync",         no_argument,       NULL, 's'},
		{"file",         required_argument, NULL, 'f'},
		{"debug",        required_argument, NULL, 'v'},
		{"threshold",    required_argument, NULL, 't'},
		{"noise",        required_argument, NULL, 'n'},
		{"snr",          required_argument, NULL, 'N'},
		{"window",       required_argument, NULL, 'W'},
		{"change-time",  required_argument, NULL, 'C'},
		{"leap-second",  required_argument, NULL, 'l'},
		{"rate",         required_argument, NULL, 'r'},
		{"mono",         no_argument,       NULL, 'm'},
		{"stereo",       no_argument,       NULL, 'M'},
		{"card",         required_argument, NULL, 'c'},
		{"iso",          no_argument,       NULL, 'I'},
		{"binary",       no_argument,       NULL, 'b'},
		{"timeout",      required_argument, NULL, 'T'},
		{"logfile",      required_argument, NULL, 'L'},
		{"repeat",       required_argument, NULL, 'R'},
		{"warranty",     no_argument,       NULL, 'w'},
		{"version",      no_argument,       NULL, 'V'},
		{"help",         no_argument,       NULL, 'h'},
		{0, 0, 0, 0}};


  while((choice = getopt_long(argc, argv, "dypkoeEa:sf:v:t:N:W:n:C:l:c:mMr:D:R:T:L:S:bIhVw", long_options, &longindex)) != -1) {
    switch (choice) {
      case 'd': options.SRCaction |= 1;		// SRCaction=1	=>	DECODE!
		break;
      case 'y': options.sys_sync = true;
		options.SRCaction |= 1;
		break;
      case 'p': options.SRCaction |= 2;		// SRCaction=2	=>	PLAY!
		break;
      case 'k': options.random_theta = true;
		options.SRCaction |= 2;
		break;
      case 'o': options.random_samples = true;
		options.SRCaction |= 2;
		break;
      case 'S': options.setDate = optarg;
		options.SRCaction |= 2;
		break;
      case 'e': options.dst = 1;
		options.SRCaction |= 2;
		break;
      case 'E': options.dst = 2;
		options.SRCaction |= 2;
		break;
      case 'a': options.power = atof(optarg);
		options.SRCaction |= 2;
		break;
      case 's': options.do_sync = true;
                break;
      case 'f': options.fo = optarg;
		break;
      case 'v': options.verb = atoi(optarg);
		break;
      case 't':	options.th = atof(optarg);
		options.SRCaction |= 1;
		break;
      case 'N': options.snr_level = atof(optarg);
		options.SRCaction |= 1;
		break;
      case 'W': options.wds = atoi(optarg);
		options.SRCaction |= 1;
		break;
      case 'n': options.noise = atof(optarg);
		options.SRCaction |= 2;
		break;
      case 'C': options.chdate = atoi(optarg);
		if((options.chdate < 0) || (options.chdate > 7)) {
		  cerr <<"Errore: errore di inserimento della data di modifica! Il parametro deve essere compreso nell'intervallo [0-7]\n";
		  options.chdate = 7;	// no changes
		}
		options.SRCaction |= 2;
		break;
      case 'l': options.leap = atoi(optarg);
		if((options.leap != 0) && (options.leap != -1) && (options.leap != 1)) {
		  cerr <<"Errore: il secondo intercalare deve essere 1 o -1 (nessun secondo intercalare => 0)\n";
		  options.leap = 0;
		}
		options.SRCaction |= 2;
		break;
      case 'r': options.fc = atoi(optarg);
		if((options.fc < 8000) || (options.fc > 48000)) {
		  cerr <<"Errore: la frequenza di campionamento deve essere compresa tra 8000 e 48000 Hz\nDefault: 8000 Hz\n";
		  options.fc = 8000;
		}
		break;
      case 'm': options.channels = 1;
		break;
      case 'M': options.channels = 2;
		break;
      case 'c': options.soundDev = optarg;
		break;
      case 'D': options.delay = atol(optarg);
		options.SRCaction |= 1;
		break;
      case 'I': options.iso = true;
		break;
      case 'b': options.binary = true;
		break;
      case 'T': options.timeout = atoi(optarg);
		if(options.timeout < 2) {
		  cerr <<"Errore: il timeout non può essere inferiore a 2 secondi. Impostazione predefinita -> 300 secondi\n";
		  options.timeout = 300;
		}
		options.SRCaction |= 1;
		break;
      case 'L': options.logfile = optarg;
		break;
      case 'R': options.repeat = atoi(optarg);
		break;
      case 'w': print_warranty();
		break;
      case 'V': cout <<"Versione: " <<VERSION <<'\n';
		break;
      case '?': // unrecognized argument
		break;
      case 'h': print_help(argv[0]);
		return 0;
      default:	cerr <<"Errore: argomento sbagliato!\n";
		print_help(argv[0]);
		return 1;
    }
  }


  if(options.verb >= 4) {
    cout <<"Opzione passata:\n\n"
	 <<"decodeSRC: " <<options.SRCaction <<'\n'
	 <<"Window Decision System length: " <<options.wds <<'\n'
	 <<"random_theta: " <<options.random_theta <<'\n'
	 <<"random_samples: " <<options.random_samples <<'\n'
	 <<"System sincronisation: " <<options.sys_sync <<'\n'
	 <<"Power level: " <<options.power <<" dB\n"
	 <<"Do sync: " <<options.do_sync <<'\n'
	 <<"Verbose level: " <<options.verb <<'\n'
	 <<"WDS: " <<options.wds <<'\n'
	 <<"SNR level: " <<options.snr_level <<'\n'
	 <<"Noise RMS: " <<options.noise <<'\n'
	 <<"Change date: " <<options.chdate <<'\n'
	 <<"Leap second: " <<options.leap <<'\n'
	 <<"Sampling frequency: " <<options.fc <<" Hz\n"
	 <<"Channels: " <<options.channels <<'\n'
	 <<"Repeat: " <<options.repeat <<'\n'
	 <<"Timeout: " <<options.timeout <<'\n'
	 <<"Sync delay: " <<options.delay <<'\n';

    if(options.fo) cout <<"FileStream: " <<options.fo <<'\n';
    if(options.soundDev) cout <<"Sound device: " <<options.soundDev <<'\n';
    if(options.logfile) cout <<"Logfile: " <<options.logfile <<'\n';
    if(options.setDate) cout <<"Set date: " <<options.setDate <<'\n';
    cout <<'\n';
  }

  SRC.set_verbose(options.verb);

  if(options.logfile) SRC.logOnFile(options.logfile);
  else SRC.logOnSTDOUT();

  if(options.do_sync) SRC.yes_sync();
  else SRC.no_sync();

  if((options.SRCaction != 1) && (options.SRCaction != 2)) {
    cerr <<"Errore: è necessario specificare l'opzione -d o -p\n\n";
    print_help(argv[0]);
    return 1;
  }


  do {	// repetition loop
    if(options.SRCaction == 1) {
      if(options.fo) {
        openState = SRC.open_file_input(options.fo, options.fc, options.channels);
        if(!openState) {
	  cerr <<"Errore: impossibile aprire il file di input del flusso " <<options.fo <<'\n';
	  return 1;
        }
      }
      else {
        openState = SRC.open_soundStream_input(options.fc, options.channels, options.soundDev);
        if(!openState) {
	  cerr <<"Errore: impossibile aprire il flusso di input audio. " <<SRC.get_sound_error() <<'\n';
	  return 1;
        }
      }


      SRC.set_decision_threshold(options.th);
      SRC.setWDS(options.wds, options.snr_level);
      SRC.set_timeout(options.timeout);
      SRC.decode();

      error = SRC.internalError();
      if(options.sys_sync && SRC.sincronized()) {		// syncronisation of the system clock
        struct timeval t;
        struct timezone tz;	// timezone
        tm ttmm;

        tz.tz_minuteswest = 60*(1 + int(SRC.OE()));
//      tz.tz_dsttime = DST_MET;		// central europe dst time constant. For old systems....

        ttmm = SRC.get_date_tm();		// 1) return the tm structure of the date/time
        t.tv_sec  = mktime(&ttmm);	// 2) convert the current time to the number of seconds since the "epoc"
																		// 3) set the number of microseonds. These are calculated taking into account the delay since last reading plus an uncertainty value
        t.tv_usec = SRC.getMilliseconds()*1000l + SRC.microsecDelay() + options.delay;

        error = settimeofday(&t, &tz);
        if(error == 0) cout <<"Ora di sistema sincronizzata!\n" <<SRC.dateSTD() <<'\n';
        else cerr <<"EE: Impossibile sincronizzare l'ora di sistema\n";

      }
    }
    else if(options.SRCaction == 2) {
      if(options.fo)
        SRC.open_file_output(options.fo, options.fc, options.channels);
      else
        SRC.open_soundStream_output(options.fc, options.channels, options.soundDev);

      if(SRC.get_OUTstate() == 0) {
        cerr <<"Errore nell'apertura del flusso di output!\n";
        return 1;
      }

      if(options.setDate) SRC.set(options.setDate, "%H:%M %d/%m/%Y");
      switch (options.dst) {
        case 1:	SRC.setOE(true);
		break;
        case 2: SRC.setOE(false);
		break;
        default: break;
      }

      if((options.chdate != 7) || (options.leap != 0))
        SRC.setWarnings(options.chdate, options.leap);

      SRC.play(options.power, options.random_samples, options.random_theta, options.noise);
      if(options.verb >= 1) {
        cout <<SRC.dateSTR(options.iso) <<'\n';
        if(options.binary) cout <<SRC <<'\n';
      }
    }

    if(options.verb >= 0) {
      if(options.SRCaction == 1) {
        if(SRC.OK()) {
	  cout <<SRC.dateSTR(options.iso) <<'\n';
	  if(SRC.warnings() && (options.verb >= 1)) {
	    cout <<"---------------\nAVVISI SRC:\n";
	    if(SRC.SE() != 7) cout <<"Cambio dell'ora tra " <<SRC.SE() <<" giorni\n";
	    if(SRC.SI()) cout <<"Secondo intercalare alla fine del mese: " <<SRC.SI() <<" secondi\n";
	    cout <<"---------------\n";
	  }
          if(options.binary) cout <<SRC <<'\n';
        }
        else {
  	  cerr <<"Errore di decodifica\n";
        }
      }
    }

    SRC.close_all();
    options.repeat--;
  } while(options.repeat != 0);


  return error;
}
