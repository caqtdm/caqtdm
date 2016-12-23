#include "proRetrieval.h"

static void GetActualTime(short *year, short *month, short *day, short *hour,
                          short *min, short *sec)
{
    time_t          time_val;
    struct timeb    timer;
    struct tm       *times;

    ftime(&timer);
    time_val = timer.time;
    times = localtime(&time_val);
    *year = times->tm_year + 1900;
    *month = times->tm_mon + 1;
    *day = times->tm_mday;
    *hour = times->tm_hour;
    *min = times->tm_min;
    *sec = times->tm_sec;
}

static float ExactTime(int Timef)
{
    short loghour, logday, logminute, logseconds, logmillisec;
    double daytime;

    cnvms_c(Timef, &logday, &loghour, &logminute, &logseconds, &logmillisec);
    daytime =  ((double) loghour + (double) logminute / 60.0 +
                (double) logseconds / 3600.0 + (double) logmillisec / (3600.0 *1000.0));

    return ((float) logday + (float) (daytime / 24.0));
}

void user_io(char *mess)
{
}

int GetLogShift(int startSeconds, char *dev, int *nbVal, float *Timer, float *Yvals)
{       
    int             unitCode[MAXDEV], Numdev, Numval, day, xmax, year;
    device          Device[1];
    short           annee, yearday, week, weekday;
    int             j, k, Tau, Tindx, Maxval, useStopValid;
    int             retr_time;
    float           Valids[MAXDEV];
    int             numerr[MAXDEV];
    double          now;
    char            logid[6];
    short           yearn, monthn, dayn, hourn, minn, secn;


    useStopValid = False;
    int jahr = 0;
    float *Valdev = (float *) 0;
    int *Timef = (int *) 0;
    short *Valerr = (short *) 0;

    GetActualTime(&yearn, &monthn, &dayn, &hourn, &minn, &secn);
    cvdate_c(yearn, monthn, dayn, &yearday, &week, &weekday);

    now = yearday + ((double) hourn + (double) minn/ 60.0 + (double) secn / 3600.0) / 24.0;

    //printf("now=%f dev=%s day=%d\n", now, dev, yearday);

    int startHours = startSeconds/3600;
    if((hourn - startHours) > 0) {
        day = 0;
    } else {
        day =  -(startHours/24 + 1);
    }

    Numdev = 1;
    Maxval = Numval = 3600/5 * 24 * (-day+1);  // anyhow too big, while in principle every 5 seconds
    strcpy(Device[0].Dev, dev);

    year = yearn;
    yearday = yearday + day;
    if (yearday < 1 )yearday = 1;
    cvtms_c(yearday, (short) 0, (short)0, (short)1, (short)0, &retr_time);

    if (year >= 2000) annee = year - 2000;
    else annee = year - 1900;
    jahr = year;
    strcpy(logid, "U");

    // allocate devices
    Valdev = (float *) malloc(sizeof(float) *Numdev * Maxval);
    Timef  = (int *)   malloc(sizeof(int)   *Numdev * Maxval);
    Valerr = (short *) malloc(sizeof(short) *Numdev * Maxval);

    // call gmvlog
    Tau = - 99;  // suppresses output

    // loggingserver is set by environment variable
    gmv("hipa-lgexp.psi.ch", Numdev, &Numval, logid, annee, retr_time, Device, Valdev, Timef, Valerr, Numdev, Maxval, &Tau);
    //printf("Numval=%d Numdev=%d\n", Numval, Numdev);

    for (j = 0; j < Numdev; j++) {
        int indx = j + Numdev * (Maxval - 1);
        nbVal[j] = Timef[indx]-2;
    }
    if (Numval < 5) {
        //printf("no data\n");
        return 0;
    }

    Numval -= 2;

    // go through data
    xmax = 0;
    Tindx = 0;
    for (j = 0; j < Numdev; j++) {
        numerr[j] = 0;
        Valids[j] = 0.0;

        for (k = 0; k < nbVal[j]; k++) {

            int indx = j + Numdev * k;
            int indx1 = j + Numdev * (k - 1);
            Timer[indx] = ExactTime(Timef[indx]);
            if ((k > 0) && (Timer[indx] > Timer[indx1])) {
                xmax = Timer[indx];
                Tindx = indx;
            }

            // case, we have an error
            if (Valerr[indx] < 0) {
                Yvals[indx] = Valids[j]; //error
                numerr[j]++;

                // no error
            } else {
                unitCode[j] = Valerr[indx];
                Yvals[indx] = Valdev[indx];
                Valids[j] = Yvals[indx]; // last correct value
            }
        }
    }

    for(j=0; j<Numval;j++) {
        Timer[j] = (Timer[j] - now) * 24.0; // in hours
    }

    // free devices
    if (Valdev != (float *)0) free(Valdev);
    if (Timef  != (int *)  0) free(Timef);
    if (Valerr != (short *)0) free(Valerr);
    return 1;

}
