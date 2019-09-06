lvoid Expand(){
    Double_t pi = TMath::Pi();
    Double_t r = 0.0025; //straw radius in m
    Double_t h = 1.8e-5; // wall thikness in m
    
    Double_t B = 0.00357; // bending stiffness
    Double_t A = r*r*TMath::Pi();
    Double_t k = 0.00357; //Timoshenko shear coefficient, just a guess
    Double_t G = 15000000; //shear modulus of mylar
    Double_t L = 1; //assume 1 meter straw
    Double_t g = 9.8; //acceleration of gravity

 
    
    Double_t c = 4.0 + pi*pi/2; // the constant infront of xi^2
    Double_t cc = c*B/(L*L); //the constant in front of 1/T
    
    
    
    //Define second order term
    TF1* second = new TF1("second",Form("%f/(%f*x)",c,g),0,700);
    second->Draw();
}