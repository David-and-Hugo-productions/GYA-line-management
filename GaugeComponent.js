import React from 'react';
import Gauge from 'react-gauge-component';

const CustomGauge = () => {
    return (
        <div style={{ backgroundColor: '#2e2e2e', height: '100vh', display: 'flex', justifyContent: 'center', alignItems: 'center' }}>
            <Gauge
                value={50} // Ändra detta värde för att justera nålen (0-100)
                width={300}
                height={150}
                label="50%"
                valueLabelStyle={{ fill: '#fff', fontSize: '20px' }}
                colorSegments={[
                    { color: '#ff4a4a', stop: 20 }, // Röd (vänster)
                    { color: '#f77b00', stop: 40 }, // Orange
                    { color: '#f0ad00', stop: 60 }, // Gul
                    { color: '#b3dc00', stop: 80 }, // Ljusgrön
                    { color: '#78c800', stop: 100 }, // Grön (höger)
                ]}
                needleColor="gray"
                centralCircleColor="#2e2e2e"
                segmentSpacing={0}
            />
        </div>
    );
};

export default CustomGauge;