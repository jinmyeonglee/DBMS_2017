USE Pokemon;

SELECT name FROM Trainer WHERE hometown = 'Blue City';

SELECT name FROM Trainer WHERE hometown = 'Brown City' OR hometown = 'Rainbow City';

SELECT name, hometown FROM Trainer WHERE name LIKE 'E%' OR name LIKE'I%' OR name LIKE 'A%' OR name LIKE 'O%' OR name LIKE 'U%';

SELECT name FROM Pokemon WHERE type = 'Water';

SELECT DISTINCT type from Pokemon;

SELECT name FROM Pokemon ORDER BY name;

SELECT name FROM Pokemon WHERE name LIKE '%s';

SELECT name FROM Pokemon WHERE name LIKE '%e%s';

SELECT name FROM Pokemon WHERE name LIKE 'E%' OR name LIKE'I%' OR name LIKE 'A%' OR name LIKE 'O%' OR name LIKE 'U%';

SELECT type, COUNT(id) FROM Pokemon GROUP BY type;

SELECT nickname FROM CatchedPokemon ORDER BY level DESC LIMIT 3;

SELECT AVG(level) FROM CatchedPokemon;

SELECT MAX(level) - MIN(level) FROM CatchedPokemon;

SELECT COUNT(id) FROM Pokemon WHERE name >= 'b' AND name < 'f';

SELECT COUNT(id) FROM Pokemon WHERE type != 'Water' AND type != 'Fire' AND type != 'Grass' AND type != 'Electric';

SELECT Trainer.name, Pokemon.name, nickname FROM CatchedPokemon, Trainer, Pokemon WHERE nickname LIKE '% %' AND CatchedPokemon.owner_id = Trainer.id AND CatchedPokemon.pid = Pokemon.id;

SELECT DISTINCT Trainer.name FROM Trainer, CatchedPokemon, Pokemon WHERE Pokemon.type = 'Psychic' AND CatchedPokemon.owner_id = Trainer.id AND Pokemon.id = CatchedPokemon.pid;

SELECT Trainer.name, Trainer.hometown FROM CatchedPokemon, Trainer WHERE CatchedPokemon.owner_id = Trainer.id GROUP BY owner_id ORDER BY AVG(level) DESC LIMIT 3;

SELECT Trainer.name, COUNT(CatchedPokemon.id) FROM CatchedPokemon, Trainer WHERE CatchedPokemon.owner_id = Trainer.id GROUP BY CatchedPokemon.owner_id ORDER BY COUNT(CatchedPokemon.id) DESC;

SELECT Pokemon.name, level FROM CatchedPokemon, Gym, Pokemon WHERE Gym.city = 'Sangnok City' AND CatchedPokemon.owner_id = Gym.leader_id AND Pokemon.id = CatchedPokemon.pid ORDER BY level;

SELECT Pokemon.name, count(owner_id) AS how_many_catched FROM CatchedPokemon RIGHT JOIN Pokemon ON Pokemon.id = CatchedPokemon.pid GROUP BY Pokemon.name ORDER BY count(owner_id) DESC;

SELECT name FROM Pokemon WHERE id IN (SELECT Y.after_id FROM Evolution AS X, Evolution AS Y WHERE X.before_id = 4 AND X.after_id = Y.before_id);

SELECT DISTINCT name FROM Pokemon, CatchedPokemon WHERE Pokemon.id = CatchedPokemon.pid AND Pokemon.id <= 30 GROUP BY owner_id HAVING COUNT(*) > 0 ORDER BY name;

SELECT Trainer.name, type FROM Trainer, CatchedPokemon, Pokemon WHERE CatchedPokemon.pid = Pokemon.id AND CatchedPokemon.owner_id = Trainer.id GROUP BY Trainer.name HAVING COUNT(DISTINCT type) = 1;

SELECT Trainer.name, type, COUNT(type) FROM Trainer, CatchedPokemon, Pokemon WHERE CatchedPokemon.pid = Pokemon.id AND CatchedPokemon.owner_id = Trainer.id GROUP BY Trainer.name, type;

SELECT Trainer.name, Pokemon.name, COUNT(pid) FROM Pokemon, Trainer, CatchedPokemon WHERE Pokemon.id = CatchedPokemon.pid AND CatchedPokemon.owner_id = Trainer.id GROUP BY owner_id HAVING COUNT(DISTINCT pid) = 1;

SELECT Trainer.name, Gym.city FROM Gym, CatchedPokemon, Pokemon, Trainer WHERE Trainer.id = CatchedPokemon.owner_id AND Gym.leader_id = CatchedPokemon.owner_id AND Pokemon.id = CatchedPokemon.pid GROUP BY leader_id HAVING COUNT(DISTINCT type) != 1;

SELECT Trainer.name, SUM(level) FROM Trainer JOIN Gym ON Trainer.id = Gym.leader_id LEFT JOIN CatchedPokemon ON CatchedPokemon.owner_id = Trainer.id AND level >= 50 GROUP BY leader_id;

SELECT DISTINCT Pokemon.name FROM Pokemon, CatchedPokemon, Trainer WHERE CatchedPokemon.owner_id = Trainer.id AND CatchedPokemon.pid = Pokemon.id AND hometown = 'Sangnok City' AND pid IN (SELECT pid FROM CatchedPokemon, Trainer WHERE CatchedPokemon.owner_id = Trainer.id AND hometown = 'Blue City');

SELECT Pokemon.name AS Final_Pokemon FROM Evolution, Pokemon WHERE Evolution.after_id = Pokemon.id AND after_id NOT IN (SELECT before_id FROM Evolution);

