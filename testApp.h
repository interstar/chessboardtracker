#ifndef _TEST_APP
#define _TEST_APP

#include "ofMain.h"
#include "ofxMaxim.h"
#include "ofxDirList.h"
#include "ofxOpenCv.h"

#define _USE_LIVE_VIDEO		// uncomment this to use a live camera
								// otherwise, we'll use a movie file


class testApp : public ofBaseApp{

	public:
		~testApp();/* deconsructor is very useful */
		void setup();
		void update();
		void draw();
        void drawSequenceGrid(int x1, int y1, int wide, int high);

		void keyPressed  (int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);

        #ifdef _USE_LIVE_VIDEO
		  ofVideoGrabber 		vidGrabber;
		#else
		  ofVideoPlayer 		vidPlayer;
		#endif

        ofxCvColorImage		colorImg;

        ofxCvGrayscaleImage 	grayImage;
		ofxCvGrayscaleImage 	grayBg;
		ofxCvGrayscaleImage 	grayDiff;

        ofxCvContourFinder 	contourFinder;

		int 				threshold;
		bool				bLearnBakground;
		bool                bRecal;

	    int left, right, top, bottom;
	    int lblob, rblob, tblob, bblob;	// which blob defined them


        int sequences[8][8];
        int steps[10];

        int phasorPosition, oldPhasorPosition;
        bool testCamera;

        int numTracks, numSteps;

        void audioRequested 	(float * input, int bufferSize, int nChannels); /* output method */
        void audioReceived 	(float * input, int bufferSize, int nChannels); /* input method */

        int		initialBufferSize; /* buffer size */
        int		sampleRate;

        /* stick you maximilian stuff below */

        double samples[8], outputs[2];
        ofxMaxiOsc waveControl;
        ofxMaxiMix mymix;
        ofxMaxiSample beats[8];

        ofxMaxiOsc trackCount[8];

        ofxDirList DIR;

        ofPoint topLeft, topRight, bottomLeft, bottomRight;

};

#endif
