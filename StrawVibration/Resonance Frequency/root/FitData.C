//Author: Jason Bono




void FitData(){

    // Set some global styling options
    gStyle->SetTitleSize(0.05);
    gStyle->SetTitleYSize(0.05);
    gStyle->SetTitleYOffset(0.7);
    gStyle->SetTitleXOffset(0.93);
    gStyle->SetOptStat(0);
    gStyle->SetOptFit(1);
    //gStyle->SetErrorX(0);
    
    // Open the file
    const int nfiles = 6;
    TCanvas *can[nfiles];
    TCanvas *can_err[nfiles];
    TCanvas *can_res[nfiles];
    TString filename[nfiles] = {"1335","1127","1000","873","744","492"};
    float Kvals[nfiles] = {};
    float Cvals[nfiles] = {};
    float eKvals[nfiles] = {};
    float eCvals[nfiles] = {};
    
    double Cmeas = 0.00376582261; //might want to take this out at some point
    double Kmeas = 156.52; // might want to take this out at some point
    double Ktheory = 156.52; //from measurments
    double Kdavid = 143.65; //davids average
    double Ctheory = 3.06660652692; //from measurments
    double Cdavid = 6.18; //davids average
    double Kavg = 156.5; //davids average
    double Cavg = 3.58; //from measurments
//    double Kstring = 161.456273508; //this includes the air inside but does not include virtual mass
    double Kstring = 166.880133526; // include no air effects at all.
    double mu_string = 3.51898e-4; // include no air effects at all.
    
//    MyFuncAvg->SetParameter(0,156.5);
//    MyFuncAvg->SetParameter(1,3.58);

    
    for (int k=0;k<nfiles;k++) {
        cout <<filename[k]<<endl;
    

  
        ifstream file;
        file.open(filename[k], ifstream::in);
    
        // Declare the variables to read in
        string line;
        float raw_len = -1;
        float raw_ten = -1;
        float raw_freq = -1;


        // Declare vectors for storing the information to be plotted
        std::vector<float> lens;
        std::vector<float> tens;
        std::vector<float> freqs;
        std::vector<float> lens_e;
        std::vector<float> tens_e;
        std::vector<float> freqs_e;
    
        float dummy = 0;
        int n = 0;
    
        // Open the file and parse each row to a "line"
        while (std::getline(file, line)) {
            // If the file is bad, break
            if (!file){ break; cout <<"the file is bad"<<endl;}
            //cout << line << endl;
            // Convert the line into a stringstream and parse it
            std::stringstream ss(line);
            if (ss >> raw_len >> raw_ten >> raw_freq) //this ignores lines of a different format
            {
                //Store vars for plotting
                if(raw_ten > 0){
                    
                    dummy = raw_len/1000.0;//convert to m
                    lens.push_back(dummy);
                    dummy = raw_ten/1000.0; //convert to kg
                    tens.push_back(dummy);
                    dummy = raw_freq;
                    freqs.push_back(dummy);
            
                    lens_e.push_back(3/1000.0); //3 mm uncertainty
                    tens_e.push_back(10/1000.0); //10g uncertainty
                    freqs_e.push_back(0.5); //.1 hz uncertainty
                    n++; // Increment the line counter
                    cout <<"Looks Good: " <<endl;
                    cout << raw_len << " " << raw_ten << " " << raw_freq << endl;
                }
                else {cout <<"skipping filtered line"<<endl;}
            }
            else {cout <<"Warning! Different line format than expected, skipping line: " <<endl;
                cout << raw_len << " " << raw_ten << " " << raw_freq << endl;}
        }
    
        std::vector<int> zeros(n, 0); // assume no error in time measurment
    
        //graph and fit
        can[k] = new TCanvas(Form("can%i",k),Form("can%i",k),800,600);
        can[k]->cd(0);
        can[k]->SetGrid();
        //TCanvas *cp = new TCanvas("cp","cp");
        //cp->SetGrid();
        TGraphErrors *grp = new TGraphErrors(n);
        for (int i=0; i<n; ++i) {
            grp->SetPoint(i,tens[i],freqs[i]);
            grp->SetPointError(i,tens_e[i],freqs_e[i]);
            cout <<tens_e[i]<<endl;
        }
        TF1 *ftp = new TF1("ftp","pol1");
        //grp->Fit("ftp");
        //c->cd(1);
        //grp->Draw("ap");

        float FrictionFactor = 0.983; //from direct measurments using a load cell
        
//        Toffset = 8.0/1000.0;
//        FrictionFactor = 0.999;
        float tmin = 0;
        float tmax = 2;
        int strawlength = lens[0]*1000; //put title in mm (converted from m)
        float L = strawlength/1000.0; //put in meters
        float Cs = 8.9e-3; //straw stretch coefficent for non prestretched straws
        float Lextra = 0.0/1000.0;
    
        TF1* Leff = new TF1("Leff",Form("%f*(1 + %f*x) + %f",L,Cs,Lextra),tmin,tmax); //stretched length
        TF1* MassCor = new TF1("MassCor",Form("sqrt(1 + %f*x)",Cs));
        float Toffset = 15.0/1000.0; //needed for the below function
        TF1* fToffset = new TF1("fToffset",Form("(%f/%f) - (%f/2.0)",Toffset,L,Toffset));//The screw term decreses with length, while the offset term was found by the loadcell
        
//        TF1* MyFit = new TF1("MyFit",Form("MassCor*[0]*sqrt(%f*x + %f)/(2*Leff) + [1]/(Leff*Leff)",FrictionFactor,Toffset));
        TF1* MyFit = new TF1("MyFit",Form("MassCor*[0]*sqrt(%f*x + fToffset)/(2*Leff) + [1]/(Leff*Leff)",FrictionFactor));

//        MyFit->SetParLimits(1,3.06,3.069);

        
        MyFit->SetParName(0,"K");
        MyFit->SetParName(1,"C");
        MyFit->SetLineColor(1);
        grp->Fit("MyFit");
        grp->Draw("ap");
        grp->SetMarkerStyle(8);
        grp->SetMarkerStyle(8);
        grp->SetMarkerSize(0.5);
        grp->SetTitle("");
        grp->GetXaxis()->SetTitle("Tension Mass (Kg)");
        grp->GetYaxis()->SetTitle("Peak Frequency (Hz)");
        grp->GetXaxis()->CenterTitle();
        grp->GetYaxis()->CenterTitle();
        grp->SetTitle(Form("Relaxed Straw Length = %i mm ",strawlength));
        //can[k]->Print(Form("plot%i.pdf",k));
        
        Kvals[k] = MyFit->GetParameter(0);
        Cvals[k] = MyFit->GetParameter(1);
        eKvals[k] = MyFit->GetParError(0);
        eCvals[k] = MyFit->GetParError(1);

//        TF1* MyFuncAvg = new TF1("MyFuncAvg",Form("[0]*sqrt(%f*x + %f)/(2*Leff) + [1]/(Leff*Leff)",FrictionFactor,Toffset));
        TF1* MyFuncAvg = new TF1("MyFuncAvg",Form("MassCor*[0]*sqrt(%f*x + fToffset)/(2*Leff) + [1]/(Leff*Leff)",FrictionFactor));
        
        MyFuncAvg->SetParameter(0,156.5);
        MyFuncAvg->SetParameter(1,3.58);
        MyFuncAvg->SetLineColor(8);
        MyFuncAvg->SetLineStyle(1);
        MyFuncAvg->Draw("Same");


        
//        TF1* MyFuncTheory = new TF1("MyFuncTheory",Form("[0]*sqrt(%f*x + %f)/(2*Leff) + [1]/(Leff*Leff)",FrictionFactor,Toffset));
        TF1* MyFuncTheory = new TF1("MyFuncTheory",Form("MassCor*[0]*sqrt(%f*x + fToffset)/(2*Leff) + [1]/(Leff*Leff)",FrictionFactor));
        MyFuncTheory->SetParameter(0,Ktheory);
        MyFuncTheory->SetParameter(1,Ctheory);
        MyFuncTheory->SetLineColor(9);
        MyFuncTheory->SetLineStyle(9);
        MyFuncTheory->Draw("Same");
        
//        TF1* MyFuncDavid = new TF1("MyFuncDavid",Form("[0]*sqrt(%f*x + %f)/(2*Leff) + [1]/(Leff*Leff)",FrictionFactor,Toffset));
        TF1* MyFuncDavid = new TF1("MyFuncDavid",Form("MassCor*[0]*sqrt(%f*x + fToffset)/(2*Leff) + [1]/(Leff*Leff)",FrictionFactor));
        MyFuncDavid->SetParameter(0,Kdavid);
        MyFuncDavid->SetParameter(1,Cdavid);
        MyFuncDavid->SetLineColor(46);
        MyFuncDavid->SetLineStyle(4);
        MyFuncDavid->Draw("Same");
        
        TF1* MyFuncString = new TF1("MyFuncString",Form("[0]*sqrt(%f*x + fToffset)/(2*Leff)",FrictionFactor));
        MyFuncString->SetParameter(0,Kstring);
//        MyFuncString->SetParameter(1,Cdavid);
        MyFuncString->SetLineColor(6);
        MyFuncString->SetLineStyle(3);
        MyFuncString->Draw("Same");


        
        
        TLegend *legFvT = new TLegend(0.6, 0.11, 0.87, 0.3);
        legFvT->AddEntry(grp, "Newest Data", "pl");
        legFvT->AddEntry(MyFit, "Newest Data Fit", "l");
        legFvT->AddEntry(MyFuncAvg, "Average of all fits", "l");
        legFvT->AddEntry(MyFuncTheory, "Theory Prediction", "l");
        legFvT->AddEntry(MyFuncDavid, "David's Empirical Prediction", "l");
        legFvT->AddEntry(MyFuncString, "Simple String Prediction", "l");
        //legK->AddEntry(fKmeas, "Measured", "l");
        //legFvT->AddEntry(fKdavid, "David's Empirical Prediction", "l");
        legFvT->Draw();

        ///-----------------------------------------------------------
        //now the error plots (error in frequency)
        can_err[k] = new TCanvas(Form("can_err%i",k),Form("can_err%i",k),800,600);
        can_err[k]->cd(0);
        can_err[k]->SetGrid();
        TF1* fErrorTheory = new TF1("fErrorTheory","MyFuncTheory - MyFit");
        TF1* fErrorAvg = new TF1("fErrorAvg","MyFuncAvg - MyFit");
        TF1* fErrorDavid = new TF1("fErrorDavid","MyFuncDavid - MyFit");
        TF1* fErrorString = new TF1("fErrorString","MyFuncString - MyFit");
        
        fErrorDavid->SetTitle(Form("Frequency Residual (for L=%i)",strawlength));
        fErrorDavid->GetXaxis()->SetTitle("Tension (Kg)");
        fErrorDavid->GetYaxis()->SetTitle("Frequency Residual (Hz)");
        fErrorDavid->GetXaxis()->CenterTitle();
        fErrorDavid->GetYaxis()->CenterTitle();
        fErrorTheory->SetLineColor(9);
        fErrorAvg->SetLineColor(8);
        fErrorDavid->SetLineColor(46);
        fErrorString->SetLineColor(6);
        fErrorTheory->SetLineStyle(9);
        fErrorAvg->SetLineStyle(1);
        fErrorDavid->SetLineStyle(4);
        fErrorString->SetLineStyle(3);
        fErrorTheory->SetLineWidth(4);
        fErrorAvg->SetLineWidth(4);
        fErrorDavid->SetLineWidth(4);
        fErrorString->SetLineWidth(4);
        fErrorDavid->Draw();
        fErrorAvg->Draw("same");
        fErrorTheory->Draw("same");
        fErrorString->Draw("same");
        
        TLegend *legerr = new TLegend(0.55, 0.7, 0.87, 0.9);
        legerr->AddEntry(fErrorTheory, "Theory", "l");
        legerr->AddEntry(fErrorAvg, "Average of all fits", "l");
        legerr->AddEntry(fErrorDavid, "David's Empirical Prediction", "l");
        legerr->AddEntry(fErrorString, "Simple String Prediction", "l");
        legerr->Draw("Same");
        
        
        //now the residual plots (error in tension)
        can_res[k] = new TCanvas(Form("can_res%i",k),Form("can_res%i",k),800,600);
        can_res[k]->cd(0);
        can_res[k]->SetGrid();
        
        TF1* fTheoryDeriv = new TF1("fTheoryDeriv",Form("%f*[0]/(4*Leff*sqrt(%f*x + %f))",FrictionFactor,FrictionFactor,Toffset));
        fTheoryDeriv->SetParameter(0,Ktheory);
        TF1* fTheoryRes = new TF1("fTheoryRes","1000*fErrorTheory/fTheoryDeriv");
        
        
        TF1* fAvgDeriv = new TF1("fAvgDeriv",Form("%f*[0]/(4*Leff*sqrt(%f*x + %f))",FrictionFactor,FrictionFactor,Toffset));
        fAvgDeriv->SetParameter(0,Kavg);
        TF1* fAvgRes = new TF1("fAvgRes","1000*fErrorAvg/fAvgDeriv");

        
        TF1* fDavidDeriv = new TF1("fDavidDeriv",Form("%f*[0]/(4*Leff*sqrt(%f*x + %f))",FrictionFactor,FrictionFactor,Toffset));
        fDavidDeriv->SetParameter(0,Kdavid);
        TF1* fDavidRes = new TF1("fDavidRes","1000*fErrorDavid/fDavidDeriv");

        TF1* fStringDeriv = new TF1("fStringDeriv",Form("1/(4*Leff*sqrt((%f + %f*x)*[0]))",Toffset,FrictionFactor));
        //fStringDeriv->SetParameter(0,Kstring);
        fStringDeriv->SetParameter(0,mu_string);
        TF1* fStringRes = new TF1("fStringRes","1000*fErrorString/fDavidDeriv");
        
        
        fDavidRes->SetTitle(Form("Tension Residual (for L=%i)",strawlength));
        fDavidRes->GetXaxis()->SetTitle("Applied Tension (Kg)");
        fDavidRes->GetYaxis()->SetTitle("Tension Residual (g)");
        fDavidRes->GetXaxis()->CenterTitle();
        fDavidRes->GetYaxis()->CenterTitle();
        fTheoryRes->SetLineColor(9);
        fAvgRes->SetLineColor(8);
        fDavidRes->SetLineColor(46);
        fStringRes->SetLineColor(6);
        fTheoryRes->SetLineStyle(9);
        fAvgRes->SetLineStyle(1);
        fDavidRes->SetLineStyle(4);
        fStringRes->SetLineStyle(3);
        fTheoryRes->SetLineWidth(4);
        fAvgRes->SetLineWidth(4);
        fDavidRes->SetLineWidth(4);
        fStringRes->SetLineWidth(4);
        fDavidRes->Draw();
        fAvgRes->Draw("same");
        fTheoryRes->Draw("same");
        fStringRes->Draw("same");
        
        
        
        
        

        
        
        
        
        
        TLegend *legres = new TLegend(0.55, 0.7, 0.87, 0.9);
        legres->AddEntry(fTheoryRes, "Theory", "l");
        legres->AddEntry(fAvgRes, "Average of all fits", "l");
        legres->AddEntry(fDavidRes, "David's Empirical Prediction", "l");
        legres->AddEntry(fStringRes, "Simple String Prediction", "l");
        legres->Draw("Same");
        
        
        
        
//        fTheoryRes->SetTitle(Form("Tension Residual (for L=%i)",strawlength));
//        fTheoryRes->GetXaxis()->SetTitle("Applied Tension (Kg)");
//        fTheoryRes->GetYaxis()->SetTitle("Tension Residual (g)");
//        fTheoryRes->GetXaxis()->CenterTitle();
//        fTheoryRes->GetYaxis()->CenterTitle();
//        fTheoryRes->Draw();
        //fTheoryRes->Draw();
        
        
    }
    float NominalL[nfiles]={133.5, 112.7, 100.0, 87.3,74.4,49.2}; //in cs

    TCanvas *cK = new TCanvas("cK","K Par",200,10,700,500);
    cK->SetGrid();
    TGraphErrors *grK = new TGraphErrors(nfiles,NominalL,Kvals,0,eKvals);
    TF1* Kfit = new TF1("Kfit","pol0");
    Kfit->SetLineColor(36);
    Kfit->SetLineStyle(1);
    Kfit->SetLineWidth(3);
    Kfit->SetParName(0,"K Average:");
    grK->Fit("Kfit","q");
    grK->SetLineColor(36);
    grK->SetLineWidth(2);
    grK->SetMarkerColor(36);
    grK->SetMarkerStyle(21);
    grK->SetMarkerSize(1.5);
    grK->SetTitle("K Parameter");
    grK->GetXaxis()->SetTitle("L (cm)");
    grK->GetYaxis()->SetTitle("K (m s^{-1} kg^{-1/2} )");
    grK->GetXaxis()->CenterTitle();
    grK->GetYaxis()->CenterTitle();
    grK->Draw("AP");
    grK->SetMinimum(142);
    grK->SetMaximum(161);
    
    
//    double Ktheory = 156.52; //from measurments
//    double Kdavid = 143.65; //davids average

    
    TF1* fKtheory = new TF1("fKtheory",Form("%f",Ktheory),0,1000);
    fKtheory->Draw("Same");
    fKtheory->SetLineColor(32);
    fKtheory->SetLineStyle(10);
    fKtheory->SetLineWidth(3);

    TF1* fKdavid = new TF1("fKdavid",Form("%f",Kdavid),0,1000);
    fKdavid->Draw("Same");
    fKdavid->SetLineColor(46);
    fKdavid->SetLineWidth(3.5);
    fKdavid->SetLineStyle(2);
    
    TLegend *legK = new TLegend(0.6, 0.11, 0.87, 0.3);
    legK->AddEntry(grK, "Newest Data", "p");
    legK->AddEntry(Kfit, "Newest Data (Fit)", "l");
    legK->AddEntry(fKtheory, "Theory Prediction", "l");
    //legK->AddEntry(fKmeas, "Measured", "l");
    legK->AddEntry(fKdavid, "David's Empirical Prediction", "l");
    legK->Draw();
    

    
    
    TCanvas *cC = new TCanvas("cC","C Par",200,10,700,500);
    cC->SetGrid();
    TGraphErrors *grC = new TGraphErrors(nfiles,NominalL,Cvals,0,eCvals);
    TF1* Cfit = new TF1("Cfit","pol0");
    Cfit->SetLineColor(36);
    Cfit->SetLineStyle(1);
    Cfit->SetLineWidth(3);
    Cfit->SetParName(0,"C Average:");
    grC->Fit("Cfit","q");
    grC->SetLineColor(36);
    grC->SetLineWidth(2);
    grC->SetMarkerColor(36);
    grC->SetMarkerStyle(21);
    grC->SetMarkerSize(1.5);
    grC->SetTitle("C Parameter");
    grC->GetXaxis()->SetTitle("L (cm)");
    grC->GetYaxis()->SetTitle("C (m^{2} s^{-1} )");
    grC->GetXaxis()->CenterTitle();
    grC->GetYaxis()->CenterTitle();
    grC->Draw("AP");
    grC->SetMinimum(0);
    grC->SetMaximum(7);
    
    
//    double Ctheory = 3.06660652692; //from measurments
//    double Cdavid = 6.18; //davids average
    
    
    TF1* fCtheory = new TF1("fCtheory",Form("%f",Ctheory),0,1000);
    fCtheory->Draw("Same");
    fCtheory->SetLineColor(32);
    fCtheory->SetLineStyle(10);
    fCtheory->SetLineWidth(3);
    
    TF1* fCdavid = new TF1("fCdavid",Form("%f",Cdavid),0,1000);
    fCdavid->Draw("Same");
    fCdavid->SetLineColor(46);
    fCdavid->SetLineWidth(3.5);
    fCdavid->SetLineStyle(2);
    
    TLegend *legC = new TLegend(0.6, 0.11, 0.87, 0.3);
    legC->AddEntry(grC, "Newest Data", "p");
    legC->AddEntry(Cfit, "Newest Data (Fit)", "l");
    legC->AddEntry(fCtheory, "Theory Prediction", "l");
    //legC->AddEntry(fCmeas, "Measured", "l");
    legC->AddEntry(fCdavid, "David's Empirical Prediction", "l");
    legC->Draw();
    
  
    
    TCanvas *cpar = new TCanvas("cpar","Par-space",200,10,700,500);
    cpar->SetGrid();
    TGraphErrors *grP = new TGraphErrors(nfiles,Kvals,Cvals,eKvals,eCvals);
    grP->SetTitle("Newest Data");
    grP->SetLineColor(36);
    grP->SetLineWidth(2);
    grP->SetMarkerColor(36);
    grP->SetMarkerStyle(21);
    grP->SetMarkerSize(1.5);
//    grP->SetTitle("Parameter Space");
//    grP->GetXaxis()->SetTitle("K (m s^{-1} kg^{-1/2} )");
//    grP->GetYaxis()->SetTitle("C (m^{2} s^{-1} )");
//    grP->GetXaxis()->CenterTitle();
//    grP->GetYaxis()->CenterTitle();
    //grP->Draw("AP");

    
    

    float kk[1] = {Ktheory};
    float cc[1] = {Ctheory};
    TGraphErrors *grTheory = new TGraphErrors(1,kk,cc,0,0);
    grTheory->SetTitle("Theory Prediction");
    grTheory->SetLineColor(46);
    grTheory->SetLineWidth(2);
    grTheory->SetMarkerColor(32);
    grTheory->SetMarkerStyle(29);
    grTheory->SetMarkerSize(4);
    
    
    float kkk[1] = {Kdavid};
    float ccc[1] = {Cdavid};
    float ekkk[1] = {1.56};
    float eccc[1] = {0.34};
    TGraphErrors *grDavid = new TGraphErrors(1,kkk,ccc,ekkk,eccc);
    grDavid->SetTitle("David's Empirical Prediction");
    grDavid->SetLineColor(46);
    grDavid->SetLineWidth(2.5);
    grDavid->SetMarkerColor(46);
    grDavid->SetMarkerStyle(33);
    grDavid->SetMarkerSize(3.4);
    
    
//    grTheory->SetTitle("CK Parameter Space");
//    grTheory->GetXaxis()->SetTitle("K (m s^{-1} kg^{-1/2} )");
//    grTheory->GetYaxis()->SetTitle("C (m^{2} s^{-1} )");
//    grTheory->GetXaxis()->CenterTitle();
//    grTheory->GetYaxis()->CenterTitle();
    //grTheory->Draw("AP");
    
    TMultiGraph *mg = new TMultiGraph();
    mg->Add(grP,"p");
    mg->Add(grTheory,"p");
    mg->Add(grDavid,"p");
    
    mg->Draw("ap");
    mg->SetTitle("C-K-Parameter Space");
    mg->GetXaxis()->SetTitle("K (m s^{-1} kg^{-1/2} )");
    mg->GetYaxis()->SetTitle("C (m^{2} s^{-1} )");
    mg->GetXaxis()->CenterTitle();
    mg->GetYaxis()->CenterTitle();
    mg->GetXaxis()->SetLimits(140,170);
    mg->SetMinimum(0);
    mg->SetMaximum(7);
    //cpar->BuildLegend();
    cpar->Update();
    TLegend *leg = new TLegend(0.6, 0.65, 0.87, 0.9);
    leg->AddEntry(grP, "Newest Data", "p");
    leg->AddEntry(grTheory, "Theory Prediction", "p");
    leg->AddEntry(grDavid, "David's Empirical Prediction", "p");
    leg->Draw("same");
    
    
}
    

    
















////////////this is an old function below/////////////////
/*
void plotpars(){
    TCanvas *c1 = new TCanvas("c1","K Par",200,10,700,500);
    //c1->SetFillColor(42);
    c1 ->SetGrid();
    const Int_t n = 6;
    
    
    

    
//    double K[n]={155.8,152.9,154.1,155.8,151.9,151.8};
//    double eK[n]={2.129,1.953,1.913,1.853,1.748,1.976};
//    double C[n]={3.181,4.275,4.089,3.52,4.54,4.54};
//    double eC[n]={1.011,0.7904,0.6938,0.5913,0.4783,0.371};
//    double L[n]={1335,1126,1000,873,744,492};
    
        //without any offset
//    double K[n]={157.117, 154.23, 155.4, 157.1, 153.2, 153.1};
//    double eK[n]={2.14712, 1.96994, 1.929, 1.869, 1.763, 1.992};
//    double C[n]={3.18110, 4.27460, 4.089, 3.52, 4.54, 4.54};
//    double eC[n]={1.01073, 0.790431, 0.6938, 0.5914, 0.4784, 0.3709};
//    double L[n]={1335, 1127, 1000, 873,744,492};
    
    //with the 7 g offset
    double K[n]={159.2, 156.6, 157.5, 156.4, 155, 154.6};
    double eK[n]={2.136, 1.961, 1.93, 1.874, 1.762, 2.005};
    double C[n]={1.731, 2.959, 3.023, 3.614, 3.809, 4.035};
    double eC[n]={1.0073, 0.7888, 0.6957, 0.5994, 0.4793, 0.3747};
    double L[n]={1335, 1127, 1000, 873,744,492};
    

    double Kavg = 153.6;
    double Kdavid = 143.65;

    for (Int_t i=0;i<n;i++) {
        L[i] = L[i]*0.1;//change to cm
    }
    
    TGraphErrors *grK = new TGraphErrors(n,L,K,0,eK);
    //grK->Fit("pol0");
    grK->SetLineColor(2);
    grK->SetLineWidth(2);
    grK->SetMarkerColor(4);
    grK->SetMarkerStyle(21);
    grK->SetTitle("K Parameter");
    grK->GetXaxis()->SetTitle("L (cm)");
    grK->GetYaxis()->SetTitle("K (m s^{-1} kg^{-1/2} )");
    grK->GetXaxis()->CenterTitle();
    grK->GetYaxis()->CenterTitle();
    grK->Draw("AP");
    grK->SetMinimum(140);
    grK->SetMaximum(165);

    TF1* fKtheory = new TF1("fKtheory",Form("%f",161.7),0,1000);
    fKtheory->Draw("Same");
    fKtheory->SetLineColor(2);
    TF1* fKmeas = new TF1("fKmeas",Form("%f",Kmeas),0,1000);
    fKmeas->Draw("Same");
    fKmeas->SetLineColor(3);
    TF1* fKavg = new TF1("fKavg",Form("%f",Kavg),0,1000);
    fKavg->Draw("Same");
    fKavg->SetLineColor(4);
    TF1* fKdavid = new TF1("fKdavid",Form("%f",Kdavid),0,1000);
    fKdavid->Draw("Same");
    fKdavid->SetLineColor(6);
    
    TLegend *legK = new TLegend(0.75, 0.11, 0.87, 0.3);
    legK->AddEntry(grK, "Data", "p");
    legK->AddEntry(fKavg, "Data Average", "l");
    legK->AddEntry(fKtheory, "Theory", "l");
    legK->AddEntry(fKmeas, "Measured", "l");
    legK->AddEntry(fKdavid, "David", "l");
    legK->Draw();

    // TCanvas::Update() draws the frame, after which one can change it
    c1->Update();
    //c1->GetFrame()->SetFillColor(21);
    c1->GetFrame()->SetBorderSize(12);
    c1->Modified();
    
    
    ///////////////////////////////////////////////////////////
    TCanvas *c2 = new TCanvas("c2","C Par",200,10,700,500);
    //c1->SetFillColor(42);
    c2 ->SetGrid();
    
    //double Cmeas = 0.00376582261;
//    double Cmeas = 3.06660652692; // from straw 2 beam deflection
//    double Cavg = 3.59908;
    
    TGraphErrors *grC = new TGraphErrors(n,L,C,0,eC);
    grC->SetLineColor(2);
    grC->SetLineWidth(2);
    grC->SetMarkerColor(4);
    grC->SetMarkerStyle(21);
    grC->SetTitle("C Parameter");
    grC->GetXaxis()->SetTitle("L (cm)");
    grC->GetYaxis()->SetTitle("C (m^{2} s^{-1} )");
    grC->GetXaxis()->CenterTitle();
    grC->GetYaxis()->CenterTitle();
    grC->Draw("AP");
    //grC->Fit("pol0");
    grC->SetMinimum(0);
    grC->SetMaximum(7);
    
    TF1* fCtheory = new TF1("fCtheory",Form("%f",3.66),0,1000);
    fCtheory->Draw("Same");
    fCtheory->SetLineColor(2);
    TF1* fCmeas = new TF1("fCmeas",Form("%f",Cmeas),0,1000);
    fCmeas->Draw("Same");
    fCmeas->SetLineColor(3);
    TF1* fCavg = new TF1("fCavg",Form("%f",4.2472),0,1000);
    fCavg->Draw("Same");
    fCavg->SetLineColor(4);
    TF1* fCdavid = new TF1("fCdavid",Form("%f",6.18),0,1000);
    fCdavid->Draw("Same");
    fCdavid->SetLineColor(6);
    
    TLegend *legC = new TLegend(0.75, 0.11, 0.87, 0.3);
    legC->AddEntry(grC, "Data", "p");
    legC->AddEntry(Cavg, "Data Average", "l");
    legC->AddEntry(Ctheory, "Theory", "l");
    legC->AddEntry(Cmeas, "Measured", "l");
    legC->AddEntry(Cdavid, "David", "l");
    legC->Draw();
    
    // TCanvas::Update() draws the frame, after which one can change it
    c2->Update();
    //c1->GetFrame()->SetFillColor(21);
    c2->GetFrame()->SetBorderSize(12);
    c2->Modified();
    
}


*/




