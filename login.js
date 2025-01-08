// Hantera inloggning
document.getElementById("loginForm").addEventListener("submit", (event) => {
    event.preventDefault();
  
    const username = document.getElementById("username").value;
    const password = document.getElementById("password").value;
  
    if (username === "d" && password === "p") {
      // Lyckad inloggning, omdirigera till developers-sidan
      window.location.href = "developers.html";
    } else {
      // Felaktigt användarnamn eller lösenord
      const errorMessage = document.getElementById("error-message");
      errorMessage.classList.remove("hidden");
    }
  });
  
  // Dölj inloggningssidan
  document.getElementById("cancelButton").addEventListener("click", () => {
    window.location.href = "index.html"; // Gå tillbaka till index.html
  });
  