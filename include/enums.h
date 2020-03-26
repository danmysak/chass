#ifndef CHASS_ENUMS_H
#define CHASS_ENUMS_H

enum Pieces {King = 0, Queen = 1, Rook = 2, Bishop = 3, Knight = 4, Pawn = 5}; // Required for packing positions
enum Sides {White = 0, Black = 1};
enum CastlingSides {Kingside, Queenside};
enum Ternary {False, Unknown, True};
enum MoveTypes {SimpleMove, Promotion, Capture, PromotionWithCapture, EnPassant, KingsideCastling, QueensideCastling};

#endif // CHASS_ENUMS_H