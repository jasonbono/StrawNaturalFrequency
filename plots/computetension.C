void computetension()
{
    

    double K = 144.93; //based off what david found
    double C = 6.18; // based off what david found
    double L = 1.13; //measured the lenth of straw in meters
    double m = .8; //weight used for tensioning in grams
    double freq = K*sqrt(m)/(2*L) + C/(L*L);
    cout <<freq<<endl;
    
    

}