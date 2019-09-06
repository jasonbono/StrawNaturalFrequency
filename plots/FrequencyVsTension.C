void FrequencyVsTension()
{
    
    gStyle->SetTitleXOffset(1);
    gStyle->SetTitleYOffset(1.3);
    //gStyle->SetTitleSize(0.09);
    //gStyle->SetTitleYSize(0.07);
    
    
    
    Double_t pi = TMath::Pi();
    //Double_t B = 0.00533; //in n.m^2, Have computed this to within ~ 30%
    Double_t B = 0.00357; //in n.m^2, Have computed this to within ~ 30%
    Double_t B = 0.00662; // as calculated for the lamina straws
    
    Double_t mu = 0.000348; //in kg/m, based off the physical measurment i did
    //Double_t mu = 0.000385; //in kg/m, this is the mass the data is tell us
    

    
    Double_t g = 9.8; //acceleration of gravity
    
    Float_t tmin = 0; //min tension in kg, not in kgf!!!
    Float_t tmax = 0.7;
    
    Float_t Cdavid = 6.18;
    Float_t Kdavid = 144.93;
    
    
    
    Double_t possion = 0.4;
    
    const int N = 7;
    Double_t mass[N] = {0.101,0.202,0.302,0.403,0.499,0.600,0.701}; //in kg
    Double_t l[N] = {1.168,0.95,0.767,0.6415,0.473,0.4,0.254}; //in meters
    Double_t freq0[N] = {23.788,32.49,40.261,46.051,50.909,54.499,58.637}; //in hz
    Double_t freq1[N] = {30.84,40.99,49.2576,56.31402,62.04,67.347,72.5};
    Double_t freq2[N] = {42.5,55,63.2,71.1,78.6,85,91.68};
    Double_t freq3[N] = {49.667,66.6,78.053,86.828,94.511,104.05,112.352};
    Double_t freq4[N] = {76.64,97.32,112.24,124.87,136.64,147.96,158.64};
    Double_t freq5[N] = {92.16,120.43794,137.5,151.37616,162.6,174.6,186.4};
    Double_t freq6[N] = {191.2,228.36,252,273.2,294.14,315.2,337};

    
    Double_t freq[N][N];
    for (int i=0; i<N; i++) {
        freq[0][i] = freq0[i];
        freq[1][i] = freq1[i];
        freq[2][i] = freq2[i];
        freq[3][i] = freq3[i];
        freq[4][i] = freq4[i];
        freq[5][i] = freq5[i];
        freq[6][i] = freq6[i];
    }
    
    TString title[N];
    title[0] = "L = 1.168 m";
    title[1] = "L = 0.950 m";
    title[2] = "L = 0.767 m";
    title[3] = "L = 0.6415 m";
    title[4] = "L = 0.473 m";
    title[5] = "L = 0.400 m";
    title[6] = "L = 0.254 m";
    
    TCanvas * can[N];
    Double_t L = 0;


    for (int i=0; i<N; i++) {

        
        
        L = l[i];
        cout<<"L = "<<L << "  l="<<l[i] <<endl;
        
        
        
        can[i] = new TCanvas(Form("can%i",i),"",750,700);
        can[i]->SetGrid();
        
        
        
        
        //Define dimensionless xi
        TF1* xi = new TF1("xi",Form("sqrt(%f/(x*%f))/%f",B,g,L),tmin,tmax);
        
        //Make stretch correction to mu
        //duke said the length increased by a factor of 0.0049 when u use 550 grams. this needs to be multipled by "x"
        //TF1* mufixed = mu*(1 + )0.0049/0.550;
        //turns out to make virtually zero difference anyway
        TF1* mufixed = new TF1("mufixed",Form("%f*(1 + 0.0049*x/0.550)",mu),tmin,tmax);
        
        //Define first natural freq of simple string
        TF1* fsnostretch = new TF1("fsnostretch",Form("sqrt(%f*x/%f)/(2*%f)",g,mu,L),tmin,tmax);
        TF1* fs = new TF1("fs",Form("sqrt(%f*x/mufixed)/(2*%f)",g,L),tmin,tmax);
        
        
        //Define the predicted frequency before approximation
        TF1* f1 = new TF1("f1",Form("fs*(1 + 2*xi + 3*xi^2)*sqrt(1 + %f^2*xi^2)",pi),tmin,tmax);
        
        //Define the predicted frequency after approximation
        TF1* f2 = new TF1("f2","fs*(1 + 2*xi)",tmin,tmax);
        
        //Define the predicted exact frequency for pinned ends
        TF1* fp = new TF1("fp",Form("fs*sqrt(1 + %f^2*xi^2)",pi),tmin,tmax);
        
        //Define the  frequency for pinned ends in the approximation
        TF1* fp2 = new TF1("fp2",Form("fs*(1 + %f^2*xi^2/2)",pi),tmin,tmax);
        
        //Define the na62 stupid function
        TF1* na26 = new TF1("na62",Form("fs*sqrt(1 + 4*%f/%f)",possion,pi),tmin,tmax);
        

        //Define davids function
        TF1* dave = new TF1("dave",Form("%f*sqrt(x)/(2*%f) + %f/(%f*%f)",Kdavid,L,Cdavid,L,L),tmin,tmax);

        //Define davids function
        //TF1* jason = new TF1("jason",Form("fs + [0]/(%f*%f)",L,L),tmin,tmax);
        //TF1* jason = new TF1("jason",Form("fs*sqrt(1 + [0]/((%f*%f)))",L,L),tmin,tmax);

        f1->SetLineColor(2);
        f1->SetLineStyle(9);
        f1->SetTitle(title[i]);
        f1->Draw();
        TGraph* gf1 = new TGraph(f1);
        gf1->GetXaxis()->SetTitle("Tension (Kg)");
        gf1->GetYaxis()->SetTitle("Frequency (Hz)");
        gf1->GetXaxis()->CenterTitle();
        gf1->GetYaxis()->CenterTitle();
        gf1->SetMinimum(0);
        gf1->SetMaximum(90 + 3*i*i + 1*i*i*i );
        gf1->Draw("AC");

        
        f2->GetXaxis()->CenterTitle();
        f2->GetYaxis()->CenterTitle();
        f2->SetLineColor(8);
        f2->SetLineStyle(1);
        TGraph* gf2 = new TGraph(f2);
        gf2->Draw("same");
        
//        TGraph* gdave = new TGraph(dave);
//        gdave->SetLineColor(1);
//        gdave->Draw("same");
        
//        fs->SetLineColor(1);
//        //fs->SetLineStyle(2);
//        fs->SetLineWidth(3);
//        TGraph* gfs = new TGraph(fs);
//        gfs->Draw("same");
//
//        fs->SetLineColor(2);
//        fs->SetLineWidth(3);
//        TGraph* gfsnostretch = new TGraph(fsnostretch);
//        gfsnostretch->Draw("same");
        
        fp->SetLineColor(4);
        fp->SetLineStyle(2);
        TGraph* gfp = new TGraph(fp);
        gfp->Draw("same");
        
        fp2->SetLineColor(7);
        fp2->SetLineStyle(7);
        TGraph* gfp2 = new TGraph(fp2);
        gfp2->Draw("same");
//
//
//        na62->SetLineColor(5);
//        na62->SetLineStyle(2);
//        TGraph* gna62 = new TGraph(na62);
//        gna62->Draw("same");
//        
//        
        TGraph* gr = new TGraph(N,mass,freq[i]);
        gr->Draw("samep");
        gr->SetMarkerStyle(22);
        //jason->SetLineColor(1);
        //gr->Fit(jason);
//
        
        
    
        
        //TMultiGraph *mg = new TMultiGraph();
        //mg->SetTitle(title[i]);
        //mg->GetXaxis()->SetTitle("Frequency (Hz)");
        //mg->GetYaxis()->SetTitle("Frequency (Hz)");
        //mg->Add(gf1,"lp");
//        mg->Add(gf2,"lp");
//        mg->Add(gfp,"cp");
//        //mg->Add(gfs,"l");
//        mg->Add(gr,"p");
        //mg->Draw("a");
        
        
        TLegend* leg = new TLegend(0.5,0.15,0.9,0.35);
        leg->AddEntry(gf1,"Glued: 1st approx","l");
        leg->AddEntry(gf2,"Glued: 2nd approx","l");
        leg->AddEntry(gfp,"Pinned: exact","l");
        leg->AddEntry(gfp2,"Pinned: approx","l");
        leg->AddEntry(gr,"Rice Data","p");
        //leg->AddEntry(gdave,"Empirical","l");
        //leg->AddEntry(gfs,"String","l");
        
        leg->Draw();

        
        
        
        
        
    }
    
    

    
    
    
    
    
    
    


//    f2->SetLineColor(3);
//    f2->SetLineStyle(5);
//    TGraph* gf2 = new TGraph(f2);
//    gf2->Draw("same");
    
    

    

    
    
    
    
    
    

    
    
}