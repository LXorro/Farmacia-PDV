-- Ejecutar como root/admin de MariaDB, p.ej.:
--   sudo mysql -u root < create_user.sql
-- Crea un usuario de aplicación con privilegios solo sobre la BD farmacia.

CREATE USER IF NOT EXISTS 'farmacia_app'@'localhost' IDENTIFIED BY 'farmacia_app_pw';
GRANT ALL PRIVILEGES ON farmacia.* TO 'farmacia_app'@'localhost';
FLUSH PRIVILEGES;
