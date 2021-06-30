#include "VCOCalibrator.h"

void VCOCalibrator::setChannel(TouchChannel *chan)
{
    channel = chan;
}

void VCOCalibrator::startCalibration()
{
    freqSampleIndex = 0;
    numSamplesTaken = 0;
    avgFreq = 0;
    sampleVCO = true;
    currState = SAMPLING_FLOOR;

    channel->output1V.resetVoltageMap();
}

void VCOCalibrator::disableCalibration()
{
    ticker.detach(); // disable ticker
}

bool VCOCalibrator::calibrateVCO()
{
    float sampledFrequency;
    int dacIndex = 0;

    // SAMPLE 1
    channel->setOctaveLed(0, TouchChannel::LedState::HIGH);
    channel->output1V.dac->write(channel->output1V.dacChannel, channel->output1V.dacVoltageMap[dacIndex]); // start at bottom most voltage.
    wait_us(100); // settling time
    sampledFrequency = this->sampleVCOFrequency();
    samples[0].first = sampledFrequency;
    samples[0].second = channel->output1V.dacVoltageMap[dacIndex];

    // Find the frequency in PITCH_FREQ array closest to the currently sampled frequency
    // use this index value later when calculating predictions
    initialPitchIndex = arr_find_closest_float(const_cast<float *>(PITCH_FREQ), NUM_PITCH_FREQENCIES, samples[0].first);

    // must be less or equal to NUM_PITCH_FREQENCIES
    if (initialPitchIndex + DAC_1VO_ARR_SIZE > NUM_PITCH_FREQENCIES)
    {
        initialPitchIndex = 0; // start at bottom 🤷‍♂️
    }

    // SAMPLE 2
    dacIndex = 12;
    channel->setOctaveLed(1, TouchChannel::LedState::HIGH);
    channel->output1V.dac->write(channel->output1V.dacChannel, channel->output1V.dacVoltageMap[dacIndex]);
    wait_us(100);
    sampledFrequency = this->sampleVCOFrequency();
    samples[1].first = sampledFrequency;
    samples[1].second = channel->output1V.dacVoltageMap[dacIndex];

    // SAMPLE 3
    dacIndex = 24;
    channel->setOctaveLed(1, TouchChannel::LedState::HIGH);
    channel->output1V.dac->write(channel->output1V.dacChannel, channel->output1V.dacVoltageMap[dacIndex]);
    wait_us(100);
    sampledFrequency = this->sampleVCOFrequency();
    samples[2].first = sampledFrequency;
    samples[2].second = channel->output1V.dacVoltageMap[dacIndex];

    // SAMPLE 3
    dacIndex = 63;
    channel->setOctaveLed(1, TouchChannel::LedState::HIGH);
    channel->output1V.dac->write(channel->output1V.dacChannel, channel->output1V.dacVoltageMap[dacIndex]);
    wait_us(100);
    sampledFrequency = this->sampleVCOFrequency();
    samples[3].first = sampledFrequency;
    samples[3].second = channel->output1V.dacVoltageMap[dacIndex];

    // now run the code
    this->generateResults();
    this->disableCalibration();
    return true;
}


// Since once frequency detection is not always accurate, take a running average of MAX_FREQ_SAMPLES
// NOTE: the first sample in freqSamples[] will always be scewed since the numSamplesTaken (since the first positive zero crossing)
// could be any value between 1 and the average sample time (between positive zero crossings)
float VCOCalibrator::calculateAverageFreq()
{
    float sum = 0;
    for (int i = 1; i < MAX_FREQ_SAMPLES; i++)
    {
        sum += this->freqSamples[i];
    }
    return sum / (MAX_FREQ_SAMPLES - 1);
}


/**
 * CALLBACK executing at desired VCO_SAMPLE_RATE_US
 * TODO: implement CircularBuffer -> https://os.mbed.com/docs/mbed-os/v5.14/apis/circularbuffer.html as it is "thread safe"
 * 
*/
float VCOCalibrator::sampleVCOFrequency()
{
    Timer timer;
    timer.start();
    freqSampleIndex = 0;
    int currTime;
    int prevTime;
    while (freqSampleIndex < MAX_FREQ_SAMPLES)
    {
        currTime = timer.read_us();
        if (currTime - prevTime >= VCO_SAMPLE_RATE_US)
        {
            // sample the ADC
            currVCOInputVal = channel->cvInput.read_u16(); // sample the ADC
            // NEGATIVE SLOPE
            if (currVCOInputVal >= (VCO_ZERO_CROSSING + VCO_ZERO_CROSS_THRESHOLD) && prevVCOInputVal < (VCO_ZERO_CROSSING + VCO_ZERO_CROSS_THRESHOLD) && slopeIsPositive)
            {
                slopeIsPositive = false;
            }
            // POSITIVE SLOPE
            else if (currVCOInputVal <= (VCO_ZERO_CROSSING - VCO_ZERO_CROSS_THRESHOLD) && prevVCOInputVal > (VCO_ZERO_CROSSING - VCO_ZERO_CROSS_THRESHOLD) && !slopeIsPositive)
            {
                float vcoPeriod = numSamplesTaken;           // how many samples have occurred between positive zero crossings
                vcoFrequency = 8000.0 / vcoPeriod;           // sample rate divided by period of input signal
                freqSamples[freqSampleIndex] = vcoFrequency; // store sample in array
                numSamplesTaken = 0;                         // reset sample count to zero for the next sampling routine
                freqSampleIndex++;                           // increment sample index by 1
                slopeIsPositive = true;
            }

            prevVCOInputVal = currVCOInputVal;
            numSamplesTaken++;
            prevTime = currTime;
        }
    }
    timer.stop();
    return calculateAverageFreq();
}

void VCOCalibrator::generateResults() {

    //The method of interpolation is considering y(x) = a + b*log(x)
    // In order to find the coefficients (a,b) that can interpolate in the best way the data set
    // we need to write a matrix equation in the form A*(a,b)'' = B

    // aux1 and aux2 variables are used to write the coefficients of matrix A
    float aux1 = 1;
    for (int i = 0; i < NUM_INTERPOLATION; i++)
    {
        aux1 = aux1 * samples[i].first;
    }
    aux1 = log(aux1);

    float aux2 = 0;
    for (int i = 0; i < NUM_INTERPOLATION; i++)
    {
        aux2 = aux2 + log(samples[i].first) * log(samples[i].first);
    }

    // we define the coeffcients of matrix A
    float A[2][2];

    A[0][0] = NUM_INTERPOLATION;
    A[0][1] = aux1;
    A[1][0] = aux1;
    A[1][1] = aux2;

    //aux3 and aux4 variables are used to write the coefficients of matrix B
    float aux3 = 0;
    for (int i = 0; i < NUM_INTERPOLATION; i++)
    {
        aux3 = aux3 + samples[i].second;
    }

    float aux4 = 0;
    for (int i = 0; i < NUM_INTERPOLATION; i++)
    {
        aux4 = aux4 + samples[i].second * log(samples[i].first);
    }

    // we define the coefficients of matrix B
    float B[2][1];
    B[0][0] = aux3;
    B[0][1] = aux4;

    // in order to calculate the inverted of matrix A we need to calculate its determinant
    float determinant;
    determinant = A[0][0] * A[1][1] - A[0][1] * A[1][0];

    // calculate the inverted of matrix A
    float A_inv[2][2];
    A_inv[0][0] = A[1][1] / determinant;
    A_inv[0][1] = -A[0][1] / determinant;
    A_inv[1][0] = -A[1][0] / determinant;
    A_inv[1][1] = A[0][0] / determinant;

    // calculate the interpolation coefficients
    float a, b;
    a = A_inv[0][0] * B[0][0] + A_inv[0][1] * B[0][1];
    b = A_inv[1][0] * B[0][0] + A_inv[1][1] * B[0][1];

    // predict DAC values for each frequency in PITCH_FREQ
    for (int i = 0; i < DAC_1VO_ARR_SIZE; i++)
    {
        float prediction = a + b * log(PITCH_FREQ[initialPitchIndex + i]);
        channel->output1V.dacVoltageMap[i] = prediction;
    }
}