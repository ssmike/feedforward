#include "network.h"
#include <cassert>
#include <random>
#include <cmath>
#include <limits>
#include <Magick++.h>
#include <fstream>
#include "ThreadPool.h"
#include <iostream>

double learning_rate = 0.03;

namespace {
    double init_link_lower_bound = -1;
    double init_link_upper_bound = 1;
    double init_neuron_lower_bound = -0.1;
    double init_neuron_upper_bound = 0.1;

    std::default_random_engine re;
    double activation_function(double x) {
        return 1/(1+exp(-x));
    }
    double activation_function_derivative(double x) {
        return activation_function(x)*(1-activation_function(x)); 
    }
}

Link::Link(Producer * source, Consumer * target): source(source), target(target) {
    std::uniform_real_distribution<double> unif(init_link_lower_bound, init_link_upper_bound);
    weight = unif(::re);
    source->addReceiver(this);
    target->addSource(this);
}

Link::Link(Producer * source, Consumer * target, double weight): Link(source, target) {
    this->weight = weight;
}

void Neuron::resetOutput() {
    this->energy_recalc = true;
}

void NetworkInput::resetOutput() {
    this->signal = 0;
}

void Neuron::addReceiver(Link * _l) {
    outputs.push_back(_l);
}

void Neuron::addSource(Link * _l) {
    inputs.push_back(_l);
}

Neuron::Neuron(double shift): Neuron(){
    this->shift = shift;
}

Neuron::Neuron() {
    std::uniform_real_distribution<double> unif(init_neuron_lower_bound, init_neuron_upper_bound);
    shift = unif(::re);
    resetOutput();
    resetError();
}

NetworkOutput::NetworkOutput(): Neuron() {
}

NetworkOutput::NetworkOutput(double x) : Neuron(x) {
}

void Neuron::resetError() {
    error_recalc = true;
}

double NetworkOutput::getState() {
    return getSignal();
}

void NetworkOutput::teach(double x) {
    error_recalc = false;
    error = (x - getSignal()) * activation_function_derivative(getSignal());
}

void Neuron::calcEnergy() {
    if (!energy_recalc) return;
    energy_recalc = false;
    double sum = 0;
    for (Link * i: inputs) {
        sum += i->getSignal();
    }
    energy = ::activation_function(sum + shift);
}

double Link::getError() {
    return weight * target->getError();
}

void Neuron::calcError() {
    if (!error_recalc) return;
    error_recalc = false;
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
    return source->getSignal() * weight;
}

double NetworkInput::getSignal() {
    return signal;
}

void NetworkInput::addReceiver(Link*){}

void Link::changeWeight() {
    weight += learning_rate * target->getError() * source->getSignal();
}

void Neuron::changeShift() {
    shift += learning_rate * error;
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
    const Magick::Geometry ImgGeometry("20x20!");
    const int hidden_layers_count = 1;
    const int hidden_layers_size = 10000;
    std::vector<NetworkInput*> inputs;
    std::vector<NetworkOutput*> outputs;
    std::vector<std::vector<Neuron*>> hidden_layers;
    std::vector<Link*> inp_links, outp_links;
    std::vector<std::vector<Link*>> mid_edges;
}

void initializeNetwork() {
    for (int i = 0; i < ::xsize * ::ysize; i++)
        inputs.push_back(new NetworkInput());
    ::hidden_layers.resize(::hidden_layers_count);
    for (int i = 0; i < hidden_layers_count; i++) {
        for (int j = 0; j < ::hidden_layers_size; j++)
            hidden_layers[i].push_back(new Neuron());
    }
    for (char x = 'a'; x <= 'z'; x++)
        ::outputs.push_back(new NetworkOutput());
    for (int i = 0; i < ::xsize * ::ysize; i++)
        for (int j = 0; j < hidden_layers[0].size(); j++)
            inp_links.push_back(new Link(inputs[i], hidden_layers[0][j]));
    mid_edges.resize(hidden_layers_count - 1);
    for (int i = 0; i < ::hidden_layers_count - 1; i++) {
        for (int j = 0; j < ::hidden_layers[i].size(); j++) {
            for (int k = 0; k < ::hidden_layers[i + 1].size(); k++) {
                mid_edges[i].push_back(new Link(hidden_layers[i][j], hidden_layers[i+1][k]));
            }
        }
    }
    for (int j = 0; j < hidden_layers[hidden_layers_count - 1].size(); j++)
        for (int i = 0; i < outputs.size(); i++) {
            outp_links.push_back(new Link(hidden_layers[hidden_layers_count - 1][j], outputs[i]));
        }
}

void readNetwork(const std::string & filename) {
    std::ifstream is(filename, std::ios::binary);  
    for (int i = 0; i < ::xsize * ::ysize; i++)
        inputs.push_back(new NetworkInput());
    hidden_layers.resize(hidden_layers_count);
    for (int i = 0; i < hidden_layers_count; i++) {
        for (int j = 0; j < hidden_layers_size; j++) {
            double tmp; is.read((char*)&tmp, sizeof(double));
            hidden_layers[i].push_back(new Neuron(tmp));
        }
    }
    for (int i = 0; i < 'z' - 'a' + 1; i++) {
        double tmp; is.read((char*)&tmp, sizeof(double));
        outputs.push_back(new NetworkOutput(tmp));
    }
    for (int i = 0; i < inputs.size(); i++) {
        for (int j = 0; j < hidden_layers_size; j++) {
            double tmp; is.read((char*)&tmp, sizeof(double));
            inp_links.push_back(new Link(inputs[i], hidden_layers[0][j], tmp));
        }
    }
    for (int i = 0; i < hidden_layers_size; i++) {
        for (int j = 0; j < outputs.size(); j++) {
            double tmp; is.read((char*)&tmp, sizeof(double));
            outp_links.push_back(new Link(hidden_layers[hidden_layers_count - 1][i], outputs[j], tmp));
        }
    }
    for (int i = 0; i < hidden_layers_count - 1; i++) {
        for (int j = 0; j < hidden_layers_size; j++) {
            for (int k = 0; k < hidden_layers_size; k++) {
                double tmp; is.read((char*)&tmp, sizeof(double));
                mid_edges[i].push_back(new Link(hidden_layers[i][j], hidden_layers[i+1][k], tmp));
            }
        }
    }
    is.close();
}

void writeNetwork(const std::string & filename) {
    std::ofstream os(filename, std::ios_base::binary);
    for (int i = 0; i < hidden_layers.size(); i++) {
        for (int j = 0; j < hidden_layers[i].size(); j++) {
            double tmp = hidden_layers[i][j]->getShift(); 
            os.write((char*)&tmp, sizeof(double));
        }
    }
    for (int i = 0; i < outputs.size(); i++) {
        double tmp = outputs[i]->getShift();
        os.write((char*)&tmp, sizeof(double));
    }
    for (int i = 0; i < inp_links.size(); i++) {
        double tmp = inp_links[i]->getWeight();
        os.write((char*)&tmp, sizeof(double));
    }
    for (int i = 0; i < outp_links.size(); i++) {
        double tmp = outp_links[i]->getWeight();
        os.write((char*)&tmp, sizeof(double));
    }
    for (int i = 0; i < mid_edges.size(); i++) {
        for (int j = 0; j < mid_edges[i].size(); j++) {
            double tmp = mid_edges[i][j]->getWeight();
            os.write((char*)&tmp, sizeof(double));
        }
    }
    os.close();
}

void resetNetwork() {
    for (int i = 0; i < hidden_layers_count; i++) {
        for (int j = 0; j < hidden_layers_size; j++) {
            hidden_layers[i][j]->resetError();
            hidden_layers[i][j]->resetOutput();
        }
    }
    for (int i = 0; i < outputs.size(); i++) {
        outputs[i]->resetError();
        outputs[i]->resetOutput();
    }
}

char runNetwork(Magick::Image& image) {
    resetNetwork();
    for (int i = 0; i < xsize; i++) {
        for (int j = 0; j < ysize; j++) {
            Magick::ColorGray color = image.pixelColor(i, j);
            inputs[xsize * i + j]->setState(color.shade());
        }
    }
    /*{
        Thread_pool<void> pool;
        
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
    }*/
    int maxi = 0;
    for (int i = 0; i < outputs.size(); i++) {
        if (outputs[i]->getSignal() > outputs[maxi]->getSignal()) 
            maxi = i;
    }
    return maxi + 'a';
}

void teachNetwork(Magick::Image& image, char c) {
    int step = 0;
    double error = 0;
    do {
        step++;
        std::cerr << runNetwork(image) << std::endl;
        error = 0;
        for (int i = 0; i < 'z' - 'a'; i++) {
            outputs[i]->teach(i == c - 'a');
            error += (outputs[i]->getSignal() - (i == c - 'a')) * (outputs[i]->getSignal() - (i == c - 'a'));
        }
        std::cerr << std::endl;
        if (error < 1e-7) break;
        for (int i = 0; i < outp_links.size(); i++) {
            outp_links[i]->changeWeight();   
        }
        for (int i = 0; i < mid_edges.size(); i++)  {
            for (int j = 0; j < mid_edges[i].size(); j++) {
                mid_edges[i][j]->changeWeight();
            }
        }
        for (int i = 0; i < hidden_layers_count; i++) {
            for (int j = 0; j < hidden_layers_count; j++) {
                hidden_layers[i][j]->changeShift();
            }
        }
        for (int i = 0; i < inp_links.size(); i++) {
            inp_links[i]->changeWeight();  
        }
        for (int i = 0; i < outputs.size(); i++) {
            outputs[i]->changeShift();
        }
        std::cerr << error << std::endl;
    } while (step < 10);
}

void prepareImage(Magick::Image & img) {
    img.type(Magick::GrayscaleType);
    img.resize(ImgGeometry);
}
