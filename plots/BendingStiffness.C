void BendingStiffness(){
    Double_t pi = TMath::Pi();
    Double_t r = 0.0025; //straw radius in m
    Double_t h = 1.8e-5; // wall thikness in m
    

    
   // Double_t I = h*h*h/(12*(1 - u*u)); //that stupid paper
    Double_t I = (pi/4.0)*(pow(r + h,4) - pow(r,4));
    Double_t E = 4.90e9; // as quoted by aseet
    Double_t mu = 3.266e-4; //linear density
    
    float totalb = 6.62709e-3;
    
    
    cout << I <<endl;
    cout << I*E <<endl;
    cout << sqrt(E*I/mu) <<endl;
    cout << sqrt(totalb/mu)<<endl;
}