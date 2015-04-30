#include "network.h"
#include <random>
#include <cmath>
#include <limits>
#include <Magick++.h>
#include <fstream>
#include "ThreadPool.h"

double learning_rate;

namespace {
    std::default_random_engine re;
    double activation_function(double x) {
        return 2/(1+exp(-x))-1;
    }
    double activation_function_derivative(double x) {
        return 2*exp(x)/(exp(x)+1)/(exp(x)+1);
    }
}

Link::Link(Producer & source, Consumer & target): source(source), target(target) {
    double init_lower_bound = -0.5;
    double init_upper_bound = 0.5;
    std::uniform_real_distribution<double> unif(init_lower_bound, init_upper_bound);
    weight = unif(::re);
    source.addReceiver(*this);
    target.addSource(*this);
}

Link::Link(Producer & source, Consumer & target, double weight): Link(source, target) {
    this->weight = weight;
}

void Neuron::resetOutput() {
    energy = std::numeric_limits<double>::quiet_NaN();
}

void NetworkInput::resetOutput() {
    this->signal = 0;
}

void Neuron::addReceiver(Link & _l) {
    outputs.push_back(&_l);
}

void Neuron::addSource(Link & _l) {
    inputs.push_back(&_l);
}

Neuron::Neuron(double shift): Neuron(){
    this->shift = shift;
}

Neuron::Neuron() {
    resetOutput();
    resetError();
}

NetworkOutput::NetworkOutput(): Neuron() {
}

NetworkOutput::NetworkOutput(double x) : Neuron(x) {
}

void Neuron::resetError() {
    error = std::numeric_limits<double>::quiet_NaN();
}

double NetworkOutput::getState() {
    return getSignal();
}

void NetworkOutput::teach(double x) {
    error = (x - getSignal()) * activation_function_derivative(getSignal());
}

void Neuron::calcEnergy() {
    if (energy == energy) return;
    double sum = 0;
    for (Link * i: inputs) {
        sum += i->getSignal();
    }
    energy = ::activation_function(sum + shift);
}

double Link::getError() {
    return weight * target.getError();
}

void Neuron::calcError() {
    if (error == error) return;
    calcEnergy();
    double sum = 0;
    for (Link * i: outputs) {
        sum += i->getError();
    }
    error = ::activation_function_derivative(energy) * sum;
}

double Neuron::getError() {
    calcError();
    return error;
}

double Neuron::getSignal() {
    calcEnergy();
    return energy;
}

void NetworkInput::setState(double x) {
    signal = x;
}

double Link::getSignal() {
    return source.getSignal() * weight;
}

double NetworkInput::getSignal() {
    return signal;
}

void NetworkInput::addReceiver(Link&){}

void Link::changeWeight() {
    weight -= learning_rate * target.getError() * source.getSignal();
}

void Neuron::changeShift() {
    shift -= learning_rate * error;
}

double Neuron::getShift() const {
    return this->shift;
}

double Link::getWeight() const {
    return this->weight;
}

namespace {
    const int xsize = 20;
    const int ysize = 20;
    const int hidden_layers_count = 1;
    const int hidden_layers_size = 10000;
    std::vector<NetworkInput> inputs;
    std::vector<NetworkOutput> outputs;
    std::vector<std::vector<Neuron>> hidden_layers;
    std::vector<Link> inp_links, outp_links;
    std::vector<std::vector<Link>> mid_edges;
}

void initializeNetwork() {
    inputs.resize(::xsize * ::ysize);
    ::hidden_layers.resize(::hidden_layers_count);
    for (int i = 0; i < hidden_layers_count; i++) {
        hidden_layers[i].resize(::hidden_layers_size);    
    }
    ::outputs.resize('z' - 'a' + 1);
    for (int i = 0; i < ::xsize * ::ysize; i++)
        for (int j = 0; j < hidden_layers[0].size(); j++)
            inp_links.push_back(Link(inputs[i], hidden_layers[0][j]));
    mid_edges.resize(hidden_layers_count - 1);
    for (int i = 0; i < ::hidden_layers_count - 1; i++) {
        for (int j = 0; j < ::hidden_layers[i].size(); j++) {
            for (int k = 0; k < ::hidden_layers[i + 1].size(); k++) {
                mid_edges[i].emplace_back(hidden_layers[i][j], hidden_layers[i+1][k]);
            }
        }
    }
    for (int i = 0; i < hidden_layers[hidden_layers_count - 1].size(); i++)
        for (int j = 0; j < outputs.size(); j++)
            inp_links.emplace_back(inputs[i], hidden_layers[hidden_layers_count - 1][j]);
}

void readNetwork(const std::string & filename) {
    std::ifstream is(filename);  
    inputs.resize(::xsize * ::ysize);
    hidden_layers.resize(hidden_layers_count);
    for (int i = 0; i < hidden_layers_count; i++) {
        for (int j = 0; j < hidden_layers_size; j++) {
            double tmp; is >> tmp;
            hidden_layers.emplace_back(tmp);
        }
    }
    for (int i = 0; i < 'z' - 'a' + 1; i++) {
        double tmp; is >> tmp;
        outputs.emplace_back(tmp);
    }
    for (int i = 0; i < inputs.size(); i++) {
        for (int j = 0; j < hidden_layers_size; j++) {
            double tmp; is >> tmp;
            inp_links.emplace_back(inputs[i], hidden_layers[0][j], tmp);
        }
    }
    for (int i = 0; i < hidden_layers_size; i++) {
        for (int j = 0; j < outputs.size(); j++) {
            double tmp; 
            is >> tmp;
            outp_links.emplace_back(hidden_layers[hidden_layers_count - 1][i], outputs[j], tmp);
        }
    }
    for (int i = 0; i < hidden_layers_count - 1; i++) {
        for (int j = 0; j < hidden_layers_size; j++) {
            for (int k = 0; k < hidden_layers_size; k++) {
                double tmp; is >> tmp;
                mid_edges[i].emplace_back(hidden_layers[i][j], hidden_layers[i+1][j], tmp);
            }
        }
    }
    is.close();
}

void writeNetwork(const std::string & filename) {
    std::ofstream os(filename);//, std::ios_base::binary);
    for (int i = 0; i < hidden_layers.size(); i++) {
        for (int j = 0; j < hidden_layers[i].size(); j++) {
            os << hidden_layers[i][j].getShift() << " ";
        }
    }
    for (int i = 0; i < outputs.size(); i++) {
        os << outputs[i].getShift() << " ";
    }
    for (int i = 0; i < inp_links.size(); i++) {
        os << inp_links[i].getWeight() << " ";
    }
    for (int i = 0; i < outp_links.size(); i++) {
        os << outp_links[i].getWeight() << " ";
    }
    for (int i = 0; i < mid_edges.size(); i++) {
        for (int j = 0; j < mid_edges[i].size(); j++) {
            os << mid_edges[i][j].getWeight() << " ";
        }
    }
    os.close();
}

void resetNetwork() {
    for (int i = 0; i < hidden_layers_count; i++) {
        for (int j = 0; j < hidden_layers_size; j++) {
            hidden_layers[i][j].resetError();
            hidden_layers[i][j].resetOutput();
        }
    }
    for (int i = 0; i < outputs.size(); i++) {
        outputs[i].resetError();
        outputs[i].resetOutput();
    }
}

char runNetwork(Magick::Image& image) {
    image.thumbnail(Magick::Geometry(xsize, ysize));
    MagickCore::SetImageColorspace(image.image(), MagickCore::GRAYColorspace);
    Magick::PixelPacket *pixels = image.getPixels(0, 0, xsize, ysize);
    resetNetwork();
    for (int i = 0; i < xsize; i++) {
        for (int j = 0; j < ysize; j++) {
            Magick::Color color = pixels[xsize * i + j];
            inputs[xsize * i + j].setState(color.intensity());
        }
    }
    {
        Thread_pool<void()> pool;
        
        for (int i = 0; i < hidden_layers_count; i++) {
            std::vector<std::future<void()>> futures;
            for (int j = 0; j < hidden_layers_size; j++) {
                pool.submit(std::bind(&Neuron::calcEnergy, hidden_layers[i][j]));
            }
            for (auto & i: futures) {
                i.get();
            }
        }
        
        for (int i = 0; i < outputs.size(); i++) {
            outputs[i].calcEnergy();
        }
    }
    int maxi = 0;
    for (int i = 0; i < outputs.size(); i++) {
        if (outputs[i].getSignal() > outputs[maxi].getSignal()) 
            maxi = i;
    }
    return maxi + 'a';
}

void teachNetwork(Magick::Image& image, char c) {
    runNetwork(image);
    for (int i = 0; i < 'z' - 'a'; i++)
        outputs[i].teach(i == c - 'a');
    for (int i = 0; i < inp_links.size(); i++) {
        inp_links[i].changeWeight();  
    }
    for (int i = 0; i < outp_links.size(); i++) {
        outp_links[i].changeWeight();   
    }
    for (int i = 0; i < mid_edges.size(); i++)  {
        for (int j = 0; j < mid_edges[i].size(); j++) {
            mid_edges[i][j].changeWeight();
        }
    }
    for (int i = 0; i < hidden_layers_count; i++) {
        for (int j = 0; j < hidden_layers_count; j++) {
            hidden_layers[i][j].changeShift();
        }
    }
    for (int i = 0; i < outputs.size(); i++) {
        outputs[i].changeShift();
    }
}
