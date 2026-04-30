namespace DP {

#include <iostream>
#include <memory>
#include <string>

// Forward declarations
class Rock;
class Paper;
class Scissors;

// Visitor interface
class MoveVisitor {
 public:
  virtual std::string visit(Rock&) = 0;
  virtual std::string visit(Paper&) = 0;
  virtual std::string visit(Scissors&) = 0;
  virtual ~MoveVisitor() = default;
};

// Element interface
class Move {
 public:
  virtual std::string accept(MoveVisitor& visitor) = 0;
  virtual ~Move() = default;
};

// Concrete Moves
class Rock : public Move {
 public:
  std::string accept(MoveVisitor& visitor) override {
    return visitor.visit(*this);
  }
};

class Paper : public Move {
 public:
  std::string accept(MoveVisitor& visitor) override {
    return visitor.visit(*this);
  }
};

class Scissors : public Move {
 public:
  std::string accept(MoveVisitor& visitor) override {
    return visitor.visit(*this);
  }
};

// Concrete Visitors (each represents "what I am" interacting with opponent)

// Rock vs *
class RockVisitor : public MoveVisitor {
 public:
  std::string visit(Rock&) override { return "Draw"; }
  std::string visit(Paper&) override { return "Lose"; }
  std::string visit(Scissors&) override { return "Win"; }
};

// Paper vs *
class PaperVisitor : public MoveVisitor {
 public:
  std::string visit(Rock&) override { return "Win"; }
  std::string visit(Paper&) override { return "Draw"; }
  std::string visit(Scissors&) override { return "Lose"; }
};

// Scissors vs *
class ScissorsVisitor : public MoveVisitor {
 public:
  std::string visit(Rock&) override { return "Lose"; }
  std::string visit(Paper&) override { return "Win"; }
  std::string visit(Scissors&) override { return "Draw"; }
};

// Game logic
std::string play(Move& player1, Move& player2) {
  // Create visitor based on player1 type
  if (dynamic_cast<Rock*>(&player1)) {
    RockVisitor v;
    return player2.accept(v);
  } else if (dynamic_cast<Paper*>(&player1)) {
    PaperVisitor v;
    return player2.accept(v);
  } else {
    ScissorsVisitor v;
    return player2.accept(v);
  }
}

int run_visitor() {
  Rock r;
  Paper p;
  Scissors s;

  std::cout << "Rock vs Scissors: " << play(r, s) << "\n";
  std::cout << "Paper vs Rock: " << play(p, r) << "\n";
  std::cout << "Scissors vs Paper: " << play(s, p) << "\n";
  std::cout << "Rock vs Rock: " << play(r, r) << "\n";

  return 0;
}

}  // namespace DP
