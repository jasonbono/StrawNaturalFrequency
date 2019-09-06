//Author: Jason Bono




void FitSimpleSupport(){

    // Set some global styling options
    gStyle->SetTitleSize(0.05);
    gStyle->SetTitleYSize(0.05);
    gStyle->SetTitleYOffset(0.7);
    gStyle->SetTitleXOffset(0.93);
    gStyle->SetOptStat(0);
    gStyle->SetOptFit(1);
    //gStyle->SetErrorX(0);
    
    // Open the file
  
    TString filename = "SimpleSupport";
    ifstream file;
    file.open(filename, ifstream::in);
    

     double Ctheory = 3.06660652692; //from bending deflection and measured mass (pluss air and virtual mass)
        
    
    // Declare the variables to read in
    string line;
    float raw_len = -1;
    float raw_freq = -1;


    // Declare vectors for storing the information to be plotted
    std::vector<float> lens;
    std::vector<float> freqs;
    std::vector<float> lens_e;
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
        if (ss >> raw_len >> raw_freq) //this ignores lines of a different format
        {
            //Store vars for plotting
            if(raw_len > 0){
                dummy = raw_len*0.0254;//convert to m from inches
                lens.push_back(dummy);
                dummy = raw_freq;
                freqs.push_back(dummy);
            
                lens_e.push_back(0.5/1000.0); //1 mm uncertainty
                freqs_e.push_back(0.5); //.5 hz uncertainty
                n++; // Increment the line counter
                cout <<"Looks Good: " <<endl;
                cout << raw_len << " " << raw_freq << endl;
            }
            else {cout <<"skipping filtered line"<<endl;}
        }
        else {cout <<"Warning! Different line format than expected, skipping line: " <<endl;
                cout << raw_len << " " << raw_freq << endl;}
    }
    
    std::vector<int> zeros(n, 0); // assume no error in time measurment
    
    //graph and fit
    TCanvas* can = new TCanvas("can","can",800,600);
    can->cd(0);
    can->SetGrid();

    TGraphErrors *grp = new TGraphErrors(n);
    for (int i=0; i<n; ++i) {
        grp->SetPoint(i,lens[i],freqs[i]);
        grp->SetPointError(i,lens_e[i],freqs_e[i]);
    }
    TF1 *ftp = new TF1("ftp","pol1");
    //grp->Fit("ftp");
    //c->cd(1);
    //grp->Draw("ap");


    float lmin = 0;
    float lmax = 1;
    float Loverhang = 0/1000.0; //straws overhang by a few mm
    TF1* MyFit = new TF1("MyFit",Form("(3.1415)/(2*(x - %f)*(x - %f))*[0]",Loverhang,Loverhang));
    TF1* fPrediction = new TF1("fPrediction",Form("(3.1415)/(2*(x - %f)*(x - %f))*%f",Loverhang,Loverhang,Ctheory),0.34,0.74);
    
    MyFit->SetParName(0,"C");
    MyFit->SetLineColor(1);
    //MyFit->SetLineWidth(3);
    grp->Fit("MyFit");
    grp->Draw("ap");
    fPrediction->SetLineColor(3);
    fPrediction->SetLineStyle(8);
    fPrediction->Draw("SAME");
//    fPrediction->Draw();

    grp->SetMarkerStyle(8);
    grp->GetXaxis()->SetTitle("Length (m)");
    grp->GetYaxis()->SetTitle("Peak Frequency (Hz)");
    grp->GetXaxis()->CenterTitle();
    grp->GetYaxis()->CenterTitle();
    grp->SetTitle("Simply Supported Straw Under Zero Tension");

    fPrediction->GetXaxis()->SetTitle("Length (m)");
    fPrediction->GetYaxis()->SetTitle("Peak Frequency (Hz)");
    fPrediction->GetXaxis()->CenterTitle();
    fPrediction->GetYaxis()->CenterTitle();
    fPrediction->SetTitle("Simply Supported Straw Under Zero Tension");
    
    
    TLegend *leg = new TLegend(0.6, 0.11, 0.87, 0.3);
    leg->AddEntry(grp, "Recent Data", "p");
    leg->AddEntry(MyFit, "Fit To Data", "l");
    leg->AddEntry(fPrediction, "Theory Prediction", "l");
    leg->Draw();
    
    
    TCanvas* canres = new TCanvas("canres","canres",800,600);
    canres->cd(0);
    canres->SetGrid();
    
    
    gStyle->SetTitleSize(0.05);
    gStyle->SetTitleYSize(0.04);
    gStyle->SetTitleYOffset(1.2);
    gStyle->SetTitleXOffset(0.93);
    gStyle->SetOptStat(0);
    gStyle->SetOptFit(1);
    TF1* fRes = new TF1("fRes","fPrediction - MyFit",0.35,0.75);
    fRes->SetLineColor(3);
    fRes->SetLineWidth(4);
    fRes->GetXaxis()->SetTitle("Length (m)");
    fRes->GetYaxis()->SetTitle("Frequency Residual (Hz)");
    fRes->GetXaxis()->CenterTitle();
    fRes->GetYaxis()->CenterTitle();
    fRes->SetTitle("Theory Residual");
    fRes->Draw();
    
    
}












