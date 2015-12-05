#ifndef DROPLET_H
#define DROPLET_H

class Droplet
{
    public:
        Droplet(int a, double v, double d, int t);

        int amplitude;
        double velocity;
        double diameter;
        int timestamp;

};

#endif //DROPLET_H
