



void AirDamping(){
    Double_t pi = TMath::Pi();
    Double_t r = 0.0025; //straw radius in m
    //r = 0.0005842;
    //r = 0.000127;
    //r = 0.0025
    
    Double_t area = r*r*TMath::Pi();
    Double_t density = 1.205; // air density in kg/m^3 at 20 C and 1 atm
    Double_t viscosity = 1.832/100000.0; // viscosity of air in kg/(m s)
    Double_t kinvis = viscosity/density; // kinematic viscosity of air
    cout << kinvis<< endl;
    
    
    cout << area <<endl;
    
    //TCanvas* can = new TCanvas("can","",800,700);
    //can->Divide(2,2);
    
     TCanvas* can1 = new TCanvas("can1","",800,700);
   // can->cd(1);
    TF1* M = new TF1("M",Form("0.5*%(%f^2)*sqrt(2.0*%f)*x/%f",r,pi,kinvis),0,150);
    M->Draw("l");
    //can->cd(2);
    TCanvas* can2 = new TCanvas("can2","",800,700);
    TF1* gamma = new TF1("gamma",Form("x*%f*%f*( (sqrt(2.0)/M) + (1/(2.0*M^2)))",density,area),0,150);
    gamma->Draw("l");
     TCanvas* can3 = new TCanvas("can3","",800,700);
    //can->cd(3);
    TF1* zetta = new TF1("zetta","sqrt(1 - (gamma/x)^2 )",0,0.5);
    zetta->SetTitle("Air's effect on frequency");
    zetta->Draw("l");
    

    zetta->SetMinimum(0);
    zetta->SetLineColor(2);
    zetta->SetLineStyle(1);
    //zetta->SetTitle("L = 1 m");
    zetta->GetXaxis()->SetTitle("Frequency (Hz)");
    zetta->GetYaxis()->SetTitle("Scaling Factor");
    zetta->GetXaxis()->CenterTitle();
    zetta->GetYaxis()->CenterTitle();
    
    
}