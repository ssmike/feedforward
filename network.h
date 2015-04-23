#ifndef NETWORK_H
#define NETWORK_H
#include <vector>

class Neuron;
class Link;

class Producer {
public:
    virtual double getSignal() = 0;
    virtual ~Producer(){}
    virtual void addReceiver(Link &) = 0;
    virtual void resetOutput() = 0;
};

class Consumer {
public:
    virtual ~Consumer(){}
    virtual void addSource(Link &) = 0;
    virtual double getError() = 0;
    virtual void resetError() = 0;
};

class Link {
private:
    double weight;
    Producer & source;
    Consumer & target;
public:
    Link(Producer&, Consumer&, double);
    Link(Producer&, Consumer&);
    double getSignal();
    double getError();
    void changeWeight();
    double getWeight() const;
};


extern double learning_rate; 

class Neuron : public Producer, public Consumer {
    protected:
        double energy, error, shift;
        std::vector<Link*> inputs;
        std::vector<Link*> outputs;
    public:
        Neuron();
        Neuron(double);
        void calcEnergy();
        void calcError();
        void resetError();
        void resetOutput();
        double getError();
        virtual void addReceiver(Link&);
        virtual void addSource(Link&);
        virtual double getSignal();
        void changeShift();
        double getShift() const;
};

class NetworkInput : public Producer {
    private:
        double signal;
    public:
        void setState(double);
        double getSignal();
        void addReceiver(Link &);
};

class NetworkOutput : public Neuron {
    private:
        double delta;
    public:
        double getState();
        void teach(double);
};

namespace std {
    class string;
}

namespace Magick {
    class Image;
}

void readNetwork(const std::string & filename);
void writeNetwork(const std::string & filename);
void initializeNetwork();
char runNetwork(Magick::Image&);
void teachNetwork(Magick::Image&, char c);

#endif
