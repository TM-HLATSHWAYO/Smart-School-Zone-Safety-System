// Smooth scroll for navigation links
document.querySelectorAll("div > ul li a").forEach(link => {
  link.addEventListener("click", function(e) {
    const href = this.getAttribute("href");
    if (href.startsWith("#")) {
      e.preventDefault();
      const targetId = href.substring(1);
      const targetElement = document.getElementById(targetId);
      if (targetElement) {
        window.scrollTo({
          top: targetElement.offsetTop - 60,
          behavior: "smooth"
        });
      }
    }
  });
});

// Highlight active link
const navLinks = document.querySelectorAll("div > ul li a");
navLinks.forEach(link => {
  link.addEventListener("click", () => {
    navLinks.forEach(l => l.classList.remove("active"));
    link.classList.add("active");
  });
});


const style = document.createElement("style");
style.innerHTML = `
  div > ul li a.active {
    border-bottom: 2px solid #3b82f6;
  }
`;
document.head.appendChild(style);
