void Timoshenko(){
    Double_t pi = TMath::Pi();
    Double_t r = 0.0025; //straw radius in m
    Double_t h = 1.8e-5; // wall thikness in m
    
    Double_t B = 0.00357; // bending stiffness 
    Double_t A = r*r*TMath::Pi();
    Double_t k = 0.00357; //Timoshenko shear coefficient, just a guess
    Double_t G = 15000000; //shear modulus of mylar
    Double_t L = 1; //assume 1 meter straw

    
 
    Double_t lessthanone = B/(k*L*L*A*G);
    cout << lessthanone <<endl;
    
}