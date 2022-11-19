INSERT INTO uservice_dynconf.users(id, login, password) VALUES('admin-id', 'admin', '1c87399feff2902bad957a2308db213f2afd4c5a26c3e4e4998b2317d2464a54');
INSERT INTO uservice_dynconf.auth_sessions(ticket, user_id) VALUES ('some_ticket', 'admin-id');
