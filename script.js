    let manualValue = null; // För manuellt inskrivet värde
    let recentValues = []; // För att lagra senaste värdena

    const TOTAL_POINTS = 120; // Antal punkter i grafen

    // Skapa grafen
    const ctx = document.getElementById('barometerGraph').getContext('2d');
    const barometerGraph = new Chart(ctx, {
      type: 'line',
      data: {
        labels: Array.from({ length: TOTAL_POINTS }, (_, i) => (i * 3).toString()),
        datasets: [{
          label: 'Medelvärde (senaste 3 sekunder)',
          data: Array(TOTAL_POINTS).fill(null),
          borderColor: 'rgb(75, 192, 192)',
          borderWidth: 2,
          fill: false,
        }],
      },
      options: {
        scales: {
          x: {
            title: {
              display: true,
              text: 'Tid (sekunder)',
            },
          },
          y: {
            title: {
              display: true,
              text: 'Värde (1-9)',
            },
            min: 1,
            max: 9,
            ticks: {
              stepSize: 1,
            },
          },
        },
      },
    });

    // Funktion för att uppdatera barometern
    function updateBarometer(value) {
      const barometer = document.getElementById('barometer');
      const barometerValue = manualValue !== null ? manualValue : value;

      // Beräkna höjden på barometern i procent
      const height = (barometerValue / 9) * 100;

      // Beräkna färgen
      let red, green;
      if (barometerValue <= 5) {
        red = Math.round(((barometerValue - 1) / 4) * 255);
        green = 255;
      } else {
        red = 255;
        green = Math.round(255 - ((barometerValue - 5) / 4) * 255);
      }
      const color = `rgb(${red}, ${green}, 0)`;

      // Uppdatera barometerns höjd och färg
      barometer.style.height = `${height}%`;
      barometer.style.backgroundColor = color;

      // Uppdatera senaste värden
      if (manualValue !== null) {
        recentValues = [barometerValue];
      } else {
        recentValues.push(barometerValue);
        if (recentValues.length > 3) {
          recentValues.shift();
        }
      }
    }

    // Funktion för att uppdatera grafen
    function updateGraph() {
      const average = recentValues.reduce((sum, value) => sum + value, 0) / recentValues.length || 0;

      const dataset = barometerGraph.data.datasets[0];
      dataset.data.push(average);
      dataset.data.shift();
      barometerGraph.update();
    }

    // Funktion för att hantera manuellt inskrivna värden
    function manualUpdateBarometer() {
      const inputValue = parseInt(document.getElementById('inputValue').value, 10);
      if (inputValue >= 1 && inputValue <= 9) {
        manualValue = inputValue;
        updateBarometer(inputValue);
        updateGraph();
      }
    }

    // Automatisk uppdatering av barometern var 1 sekund
    setInterval(() => {
      if (manualValue === null) {
        const randomValue = Math.floor(Math.random() * 9) + 1;
        updateBarometer(randomValue);
      }
    }, 1000);

    // Uppdatera grafen var 3:e sekund
    setInterval(updateGraph, 3000);

    // Startvärde
    updateBarometer(1);
  