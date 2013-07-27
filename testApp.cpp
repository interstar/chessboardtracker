#include "testApp.h"


//--------------------------------------------------------------
void testApp::setup(){


	#ifdef _USE_LIVE_VIDEO
        vidGrabber.setDeviceID(1);
        vidGrabber.setVerbose(true);
        vidGrabber.initGrabber(320,240);
	#else
        vidPlayer.loadMovie("fingers.mov");
        vidPlayer.play();
	#endif

    colorImg.allocate(320,240);
	grayImage.allocate(320,240);
	grayBg.allocate(320,240);
	grayDiff.allocate(320,240);

	bLearnBakground = true;
	threshold = 80;

    numTracks = 8;
    numSteps = 8;

    testCamera = false;
	/* This is stuff you always need.*/

	sampleRate 			= 44100; /* Sampling Rate */
	initialBufferSize	= 4096;	/* Buffer Size. you have to fill this buffer with sound*/

	for (int i=0;i<numTracks;i++) {
        steps[i]=0;
	}

    ofSetLogLevel(false);
	DIR.listDir("sounds");


	for (int i = 0; i < numTracks; i++) {
	    cout << DIR.getPath(i) << "\n";
		beats[i].load(ofToDataPath(DIR.getPath(i)));
		beats[i].getLength();

	}

    for (int i=0;i<numTracks;i++) {
        for(int j=0;j<numSteps;j++) {
            sequences[i][j]=0;
        }
    }

    sequences[1][0] = 1;
    sequences[1][2] = 1;
    sequences[1][4] = 1;
    sequences[1][6] = 1;
    sequences[5][7] = 1;
    sequences[7][2] = 1;
    sequences[3][4] = 1;


    bRecal = false;
    topLeft.x = 0; topLeft.y = 0;
    topRight.x = 320; topRight.y = 0;
    bottomRight.x = 320; bottomRight.y = 240;
    bottomLeft.x = 0; bottomLeft.y = 240;

	ofSoundStreamSetup(2, 0, this, sampleRate, initialBufferSize, 4);/* Call this last ! */

}

//--------------------------------------------------------------
void testApp::update(){
    if (!testCamera) {return;} // we only do this at the beginning of the sequence

	ofBackground(100,100,100);

    bool bNewFrame = false;

	#ifdef _USE_LIVE_VIDEO
       vidGrabber.grabFrame();
	   bNewFrame = vidGrabber.isFrameNew();
    #else
        vidPlayer.idleMovie();
        bNewFrame = vidPlayer.isFrameNew();
	#endif

	if (bNewFrame){

		#ifdef _USE_LIVE_VIDEO
            colorImg.setFromPixels(vidGrabber.getPixels(), 320,240);
	    #else
            colorImg.setFromPixels(vidPlayer.getPixels(), 320,240);
        #endif

    /*
        if (!bRecal) {
            colorImg.warpPerspective(topLeft, topRight, bottomRight, bottomLeft);
        }
    */

        grayImage = colorImg;
		if (bLearnBakground == true){
			grayBg = grayImage;		// the = sign copys the pixels from grayImage into grayBg (operator overloading)
			bLearnBakground = false;
		}

		// take the abs value of the difference between background and incoming and then threshold:
		grayDiff.absDiff(grayBg, grayImage);
		grayDiff.threshold(threshold);

        if (!bRecal) {
            grayDiff.warpPerspective(topLeft, topRight, bottomRight, bottomLeft);
            colorImg.warpPerspective(topLeft, topRight, bottomRight, bottomLeft);
        }


		// find contours which are between the size of 20 pixels and 1/3 the w*h pixels.
		// also, find holes is set to true so we will get interior contours as well....

		contourFinder.findContours(grayDiff, 20, (340*240)/3, 20, true);	// find holes

	}


    if (bRecal) {
        // recalibrate
        // we assume we found four points ... these we'll use as the four points of our callibration

        int tops[4]={0,0,0,0};
        int lefts[4]={0,0,0,0};
        int rights[4] = {0,0,0,0};
        int bottoms[4] = {0,0,0,0};
        int cxs[4], cys[4];

        for (int i=0;i<4;i++) {
            cxs[i] = contourFinder.blobs[i].centroid.x;
            cys[i] = contourFinder.blobs[i].centroid.y;
        }

        for (int i=0;i<4;i++) {
            int count[4] = {0,0,0,0};
            for (int j=0;j<4;j++) {
                if (cxs[i] < cxs[j]) {count[0]++;}
                if (cxs[i] > cxs[j]) {count[2]++;}
                if (cys[i] < cys[j]) {count[1]++;}
                if (cys[i] > cys[j]) {count[3]++;}
            }
            if (count[0]>1) { lefts[i] = 1; }
            if (count[1]>1) { tops[i] = 1; }
            if (count[2]>1) { rights[i] = 1;}
            if (count[3]>1) { bottoms[i] = 1;}
        }

        for (int i=0;i<4;i++) {
            if ((tops[i]==1) && (lefts[i]==1)) {
                topLeft.x = cxs[i];
                topLeft.y = cys[i];
                continue;
            }
            if ((tops[i]==1) && (rights[i]==1)) {
                topRight.x = cxs[i];
                topRight.y = cys[i];
                continue;
            }
            if ((bottoms[i]==1) && (lefts[i]==1)) {
                bottomLeft.x = cxs[i];
                bottomLeft.y = cys[i];
                continue;
            }
            if ((bottoms[i]==1) && (rights[i]==1)) {
                bottomRight.x = cxs[i];
                bottomRight.y = cys[i];
                continue;
            }
        }

        cout << "topLeft " << topLeft.x <<"," << topLeft.y << "\n";
        cout << "topRight " << topRight.x <<"," << topRight.y<< "\n";
        cout << "bottomLeft " << bottomLeft.x <<"," << bottomLeft.y<< "\n";
        cout << "bottomRight " << bottomRight.x <<"," << bottomRight.y<< "\n";


    } else {
        // not recalibrating

        for (int i=0;i<numTracks;i++) {
            for(int j=0;j<numSteps;j++) {
                sequences[i][j] = 0;
            }
        }


        for (int i=0; i<contourFinder.nBlobs;i++) {
            float x, y;
            x = ofMap(contourFinder.blobs[i].centroid.x, 0, 320, 0,numSteps);
            y = ofMap(contourFinder.blobs[i].centroid.y, 0, 240, 0,numTracks);

            sequences[(int)y][(int)x]=1;
        }

    }
}


void testApp::drawSequenceGrid(int x1, int y1, int wide, int high) {
    int px = x1; // pattern x and y
    int py = y1;
    int dx = wide/8;
    int dy = high/8;
    for (int i=0;i<numTracks;i++) {
	    for(int j=0;j<numSteps;j++) {
            ofSetColor(255,255,150);
            int x1 = px+j*dx;
            int y1 = py+i*dy;
            int x2 = x1+dx;
            int y2 = y1+dy;
            ofLine(x1,y1,x2,y1);
            ofLine(x1,y1,x1,y2);
            ofLine(x2,y1,x2,y2);
            ofLine(x1,y2,x2,y2);
	        if (sequences[i][j]==1) {
                ofSetColor(200,100,100);
                ofFill();
                ofRect(x1,y1,dx,dy);
	        }
            ofSetColor(50,100,255);
            int cx = px + phasorPosition * dx;
            int cy = y1+dy/2;

            ofFill();
            ofEllipse(cx+dy/2,cy,dy/2,dy/2);

        }
 	}

}
//--------------------------------------------------------------
void testApp::draw(){

	// draw the incoming, the grayscale, the bg and the thresholded difference
	ofSetColor(0xffffff);
	colorImg.draw(20,20);
	//grayImage.draw(360,20);
	grayBg.draw(20,280);
	grayDiff.draw(360,280);

	// then draw the contours:

	ofFill();
	ofSetColor(0x333333);
	ofRect(360,540,320,240);
	ofSetColor(0xffffff);

	// we could draw the whole contour finder
	//contourFinder.draw(360,540);

    int loff = 720;
    int toff = 280;

	ofSetColor(0xffffff);
    ofRectangle(loff+left,toff+top,loff+right,toff+bottom);

	// or, instead we can draw each blob individually,
	// this is how to get access to them:
    for (int i = 0; i < contourFinder.nBlobs; i++){
        contourFinder.blobs[i].draw(680,280);
    }


    drawSequenceGrid(360,20,320,240);
    //drawSequenceGrid(contourFinder.blobs[lblob].centroid.x, contourFinder)

	// finally, a report:
	ofSetColor(0xffffff);
	char reportStr[1024];
	sprintf(reportStr, "bg subtraction and blob detection\npress ' ' to capture bg\nthreshold %i (press: +/-)\nnum blobs found %i, fps: %f", threshold, contourFinder.nBlobs, ofGetFrameRate());
	ofDrawBitmapString(reportStr, 20, 600);



}




//--------------------------------------------------------------
void testApp::keyPressed  (int key){

	switch (key){
		case ' ':
			bLearnBakground = true;
			break;
        case '#' :
            bRecal = !bRecal;
            break;
		case '+':
			threshold ++;
			if (threshold > 255) threshold = 255;
			break;
		case '-':
			threshold --;
			if (threshold < 0) threshold = 0;
			break;
	}
}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){
}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){
}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){
}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void testApp::audioRequested 	(float * output, int bufferSize, int nChannels) {

    // for each sample in the buffer
    double sample_total = 0;
    oldPhasorPosition = phasorPosition;
    testCamera = false;

	for (int i = 0; i < bufferSize; i++){

        // for each tracks
        for (int j=0;j<numTracks;j++) {

            //phasor can take three arguments; frequency, start value and end value.
            phasorPosition = (int)trackCount[j].phasor(1., 0, numSteps); // (int)trackCount[j].phasor(1.,0, numSteps);
            steps[j]=sequences[j][phasorPosition];

            if (steps[j] == 1) {
                // we there's a play
                samples[j]=beats[j].play4(8.0f, 0, beats[j].length/2);
            } else {
                samples[j] = 0; // need to clear the sample value if it's not producing
                beats[j].trigger();
            }
            sample_total += samples[j];
        }

		mymix.stereo(samples[0]+samples[1]+samples[2]+samples[3]+samples[4]+samples[5]+samples[6]+samples[7], outputs, 0.5);

		output[i*nChannels    ] = outputs[0]; /* You may end up with lots of outputs. add them here */
		output[i*nChannels + 1] = outputs[1];
	}

	if (oldPhasorPosition != phasorPosition) {
            testCamera = true;
	}

}

//--------------------------------------------------------------
void testApp::audioReceived 	(float * input, int bufferSize, int nChannels){
	/* You can just grab this input and stick it in a double, then use it above to create output*/
	for (int i = 0; i < bufferSize; i++){
		/* you can also grab the data out of the arrays*/
	}
}

testApp::~testApp() {

/*	delete beat.myData; /*you should probably delete myData for any sample object
						 that you've created in testApp.h*/
	for (int i=0; i < numTracks; i++) {
		delete beats[i].myData;
	}

}
