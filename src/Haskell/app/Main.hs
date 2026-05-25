module Main where

main :: IO ()
main = putStrLn "Hello, Haskell!"
doublesmallnumber x = if x >= 100
			then x*2
			else x 
doublesmallnumber 101
