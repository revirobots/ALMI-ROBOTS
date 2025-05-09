// JavaScript para funcionalidades adicionales

// Esperar a que el DOM esté completamente cargado
document.addEventListener('DOMContentLoaded', function() {
    // 1. Loader
    setTimeout(function() {
        document.querySelector('.loader').style.display = 'none';
    }, 1500);
    
    // 2. Botón para volver arriba
    const backToTopButton = document.getElementById('backToTop');
    window.addEventListener('scroll', function() {
        if (window.pageYOffset > 300) {
            backToTopButton.classList.add('visible');
        } else {
            backToTopButton.classList.remove('visible');
        }
    });
    
    // 3. Animaciones al hacer scroll
    const scrollElements = document.querySelectorAll('.scroll-animate');
    const elementInView = (el, dividend = 1) => {
        const elementTop = el.getBoundingClientRect().top;
        return (
            elementTop <= (window.innerHeight || document.documentElement.clientHeight) / dividend
        );
    };
    
    const displayScrollElement = (element) => {
        element.classList.add('animate');
    };
    
    const hideScrollElement = (element) => {
        element.classList.remove('animate');
    };
    
    const handleScrollAnimation = () => {
        scrollElements.forEach((el) => {
            if (elementInView(el, 1.25)) {
                displayScrollElement(el);
            } else {
                hideScrollElement(el);
            }
        });
    };
    
    window.addEventListener('scroll', () => {
        handleScrollAnimation();
    });
    
    // 4. Modo oscuro
    const darkModeToggle = document.querySelector('.dark-mode-toggle');
    const darkModeIcon = document.getElementById('darkModeIcon');
    
    function toggleDarkMode() {
        document.body.classList.toggle('dark-mode');
        if (document.body.classList.contains('dark-mode')) {
            darkModeIcon.classList.remove('fa-moon');
            darkModeIcon.classList.add('fa-sun');
            localStorage.setItem('darkMode', 'enabled');
        } else {
            darkModeIcon.classList.remove('fa-sun');
            darkModeIcon.classList.add('fa-moon');
            localStorage.setItem('darkMode', 'disabled');
        }
    }
    
    // Verificar preferencia guardada
    if (localStorage.getItem('darkMode') === 'enabled') {
        document.body.classList.add('dark-mode');
        darkModeIcon.classList.remove('fa-moon');
        darkModeIcon.classList.add('fa-sun');
    }
    
    darkModeToggle.addEventListener('click', toggleDarkMode);
    
    // 5. Cambio de tamaño de texto
    function changeFontSize(action) {
        const html = document.documentElement;
        let currentSize = parseFloat(window.getComputedStyle(html).fontSize);
        
        switch(action) {
            case 'increase':
                html.style.fontSize = (currentSize * 1.1) + 'px';
                break;
            case 'decrease':
                html.style.fontSize = (currentSize * 0.9) + 'px';
                break;
            case 'reset':
                html.style.fontSize = '16px';
                break;
        }
        
        // Guardar preferencia
        localStorage.setItem('fontSize', html.style.fontSize);
        
        // Mostrar notificación
        showNotification('Tamaño de texto actualizado', 'info');
    }
    
    // Verificar preferencia guardada
    if (localStorage.getItem('fontSize')) {
        document.documentElement.style.fontSize = localStorage.getItem('fontSize');
    }
    
    // Asignar al objeto window para acceso global
    window.changeFontSize = changeFontSize;
    
    // 6. Lightbox para imágenes
    const lightbox = document.getElementById('lightbox');
    const lightboxImg = document.getElementById('lightboxImg');
    
    function openLightbox(src) {
        lightboxImg.src = src;
        lightbox.style.display = 'flex';
    }
    
    function closeLightbox() {
        lightbox.style.display = 'none';
    }
    
    // Asignar al objeto window para acceso global
    window.openLightbox = openLightbox;
    window.closeLightbox = closeLightbox;
    
    // 7. Sistema de notificaciones
    function showNotification(message, type = 'info', duration = 3000) {
        const container = document.getElementById('notificationsContainer');
        const notification = document.createElement('div');
        notification.className = `notification ${type}`;
        notification.innerHTML = `
            ${message}
            <button onclick="this.parentElement.remove()">×</button>
        `;
        container.appendChild(notification);
        
        // Auto-eliminar después de un tiempo
        setTimeout(() => {
            notification.remove();
        }, duration);
    }
    
    // Asignar al objeto window para acceso global
    window.showNotification = showNotification;
    
    // 8. Búsqueda en el sitio - Eliminada por solicitud del usuario
    
    // 9. Tabs
    function openTab(tabId) {
        // Ocultar todos los contenidos de tabs
        const tabContents = document.querySelectorAll('.tab-content');
        tabContents.forEach(content => {
            content.classList.remove('active');
        });
        
        // Desactivar todos los botones de tabs
        const tabButtons = document.querySelectorAll('.tab-button');
        tabButtons.forEach(button => {
            button.classList.remove('active');
        });
        
        // Mostrar el contenido seleccionado
        document.getElementById(tabId).classList.add('active');
        
        // Activar el botón correspondiente
        event.currentTarget.classList.add('active');
    }
    
    // Asignar al objeto window para acceso global
    window.openTab = openTab;
    
    // 10. Menú móvil mejorado
    const menuToggle = document.getElementById('menuToggle');
    const navLinks = document.getElementById('navLinks');
    
    if (menuToggle && navLinks) {
        menuToggle.addEventListener('click', function() {
            navLinks.classList.toggle('active');
            
            // Cambiar icono del botón
            const icon = menuToggle.querySelector('i');
            if (navLinks.classList.contains('active')) {
                icon.classList.remove('fa-bars');
                icon.classList.add('fa-times');
            } else {
                icon.classList.remove('fa-times');
                icon.classList.add('fa-bars');
            }
        });
        
        // Manejar dropdowns en móvil
        const dropdowns = document.querySelectorAll('.dropdown');
        dropdowns.forEach(dropdown => {
            const link = dropdown.querySelector('a');
            link.addEventListener('click', function(e) {
                if (window.innerWidth <= 768) {
                    e.preventDefault();
                    dropdown.classList.toggle('active');
                }
            });
        });
        
        // Cerrar menú al hacer clic en un enlace
        const menuLinks = navLinks.querySelectorAll('a:not(.dropdown > a)');
        menuLinks.forEach(link => {
            link.addEventListener('click', function() {
                if (window.innerWidth <= 768) {
                    navLinks.classList.remove('active');
                    menuToggle.querySelector('i').classList.remove('fa-times');
                    menuToggle.querySelector('i').classList.add('fa-bars');
                }
            });
        });
    }
});
